## =====================================================================
##  Shared simulation engine
## =====================================================================
##  Dataset-independent machinery for the two-phase left-truncation
##  experiments. Every simulation folder reports in the same format by
##  sourcing this file and supplying its own DGP.
##
##  What a simulation folder must provide (see <sim>/dgp.R):
##
##    sim$name                       character
##    sim$draw_train(setup)          -> data.frame with x-columns plus
##                                      `entry`, `time`, `status`
##    sim$draw_test(setup, m)        -> numeric matrix of covariates drawn
##                                      from the POPULATION distribution
##                                      (not the retained one)
##    sim$truth(Xtest, tgrid, setup) -> m x M matrix of true S(t|x), or
##                                      NULL when no analytic truth exists
##
##  Everything below is common to all simulations.
## =====================================================================

suppressPackageStartupMessages({
  library(survival)
  library(randomForestSRC)
  library(parallel)
})


## ---------------------------------------------------------------------
##  1. Estimators -> S-hat on the shared grid  (m x M)
## ---------------------------------------------------------------------
##  Both estimators are step functions on their own event times, so
##  findInterval maps them onto the common grid. This keeps every arm and
##  the truth on an identical time axis.

## The formula is built here but must resolve `data = ...` in the caller's
## frame: coxph evaluates its model.frame call inside environment(formula),
## which would otherwise be this function's own (empty) frame.
surv_formula <- function(xn, lhs = "Surv(time, status)") {
  f <- as.formula(paste(lhs, "~", paste(xn, collapse = " + ")))
  environment(f) <- parent.frame()
  f
}

cox_survival_matrix <- function(fit, Xtest, tgrid) {
  bh  <- survival::basehaz(fit, centered = FALSE)   # Breslow, uncentered
  idx <- findInterval(tgrid, bh$time)
  H0  <- ifelse(idx == 0, 0, bh$hazard[idx])
  b   <- coef(fit)
  lp  <- as.vector(Xtest[, names(b), drop = FALSE] %*% b)
  exp(-outer(exp(lp), H0))
}

## Both estimators' native output is a step function on their own event
## times; this maps any such (survival matrix, event-time vector) pair onto
## the shared grid. Factored out so the same mapping serves a fresh
## prediction (rsf_survival_matrix, below) and an already-computed OOB
## curve pulled straight off a fitted forest (see C-index section).
step_to_grid <- function(surv_mat, time_interest, tgrid) {
  idx <- findInterval(tgrid, time_interest)
  out <- matrix(1, nrow = nrow(surv_mat), ncol = length(tgrid))  # S=1 pre-t1
  nz  <- idx > 0
  if (any(nz)) out[, nz] <- surv_mat[, idx[nz], drop = FALSE]
  out
}

## A test subject is dropped down the trees on covariates alone, so
## prediction needs no entry times and this serves both forest arms.
rsf_survival_matrix <- function(fit, Xtest_df, tgrid) {
  pr <- predict(fit, newdata = Xtest_df)
  step_to_grid(pr$survival, pr$time.interest, tgrid)
}


## ---------------------------------------------------------------------
##  2. Scoring (pluggable)
## ---------------------------------------------------------------------
##  A scorer reports error at each grid point; the headline figure is the
##  mean of that curve, so the summary number and the curve can never
##  disagree. Datasets without an analytic truth can supply a different
##  scorer with the same two fields.

scorer_mad_vs_truth <- list(
  label   = "MAD vs true S",
  by_time = function(Shat, ctx) colMeans(abs(Shat - ctx$Strue))
)

score_arm <- function(Shat, ctx, scorer) {
  v <- scorer$by_time(Shat, ctx)
  list(by_time = v, overall = mean(v))
}


## ---------------------------------------------------------------------
##  2b. C-index (rank-based, scored on the training sample's real outcomes)
## ---------------------------------------------------------------------
##  MAD-vs-truth (above) checks whether the predicted *curve* matches the
##  true S(t|x); this checks whether the predicted *ranking* of subjects
##  matches who actually failed first, using each replication's own real
##  (time, status, entry) -- not the population test set, which has no
##  simulated outcomes at all, only covariates and a known truth curve.
##
##  Each arm's own held-out-style fit is reused, at no extra fitting cost:
##  RSF's built-in OOB survival curve, Cox's in-sample curve (Cox has no
##  OOB equivalent). Both get mapped to a single risk score per subject --
##  sum(1 - S_hat) over the shared grid, i.e. higher = worse predicted
##  survival = higher risk -- so one code path scores every arm.
##
##  Every arm is scored against the SAME real entry vector regardless of
##  whether that arm's own fit used it. That isolates one thing: does
##  giving the model entry.time at FIT time produce a better risk ranking,
##  when the EVALUATION is always done correctly (truncation-aware)? An
##  arm scored against entry=0 would just reproduce the old biased
##  evaluation this feature exists to replace.
##
##  get.cindex() returns an error rate (1 - C), matching MAD's own
##  lower-is-better convention -- so both scorers can sit in one table
##  without a sign flip.

risk_score <- function(Shat) rowSums(1 - Shat)

cindex_arm <- function(Shat_own, time, status, entry) {
  get.cindex(time, status, risk_score(Shat_own), entry = entry)
}


## ---------------------------------------------------------------------
##  3. Arms
## ---------------------------------------------------------------------
##  Four arms in two families. Within a family the two arms differ only in
##  whether the entry vector is used. The phase supplies that vector:
##  zeros in phase 1, observed entry times in phase 2. The standard arms
##  ignore it entirely.
##
##  Display names for these arms live in sim_report.R, so wording can be
##  changed without invalidating a cached run.
##
##  `d` never contains the entry column, so the standard arms cannot see
##  entry information even accidentally, and it can never drift onto the
##  right-hand side as a predictor.

build_arms <- function(hp) {
  force(hp)
  list(
    cox_std = list(
      fit  = function(d, entry, xn, seed) coxph(surv_formula(xn), data = d),
      surv = function(fit, Xtest, Xtest_df, tg) cox_survival_matrix(fit, Xtest, tg)
    ),
    cox_lt = list(
      fit  = function(d, entry, xn, seed) {
        dd <- d
        dd$.entry <- entry
        coxph(surv_formula(xn, "Surv(.entry, time, status)"), data = dd)
      },
      surv = function(fit, Xtest, Xtest_df, tg) cox_survival_matrix(fit, Xtest, tg)
    ),
    rsf_std = list(
      fit  = function(d, entry, xn, seed) {
        rfsrc(surv_formula(xn), data = d, ntree = hp$ntree, mtry = hp$mtry,
              nodesize = hp$nodesize, seed = seed)
      },
      surv = function(fit, Xtest, Xtest_df, tg) rsf_survival_matrix(fit, Xtest_df, tg)
    ),
    rsf_lt = list(
      fit  = function(d, entry, xn, seed) {
        rfsrc(surv_formula(xn), data = d, ntree = hp$ntree, mtry = hp$mtry,
              nodesize = hp$nodesize, seed = seed, entry.time = entry)
      },
      surv = function(fit, Xtest, Xtest_df, tg) rsf_survival_matrix(fit, Xtest_df, tg)
    )
  )
}

ARM_KEYS <- c("p1_cox_std", "p1_cox_lt", "p1_rsf_std", "p1_rsf_lt",
              "p2_cox_std", "p2_cox_lt", "p2_rsf_std", "p2_rsf_lt")


## ---------------------------------------------------------------------
##  4. One replication -> both phases on one sample
## ---------------------------------------------------------------------
##  Phase 1 gives every arm a zero entry vector; phase 2 gives the
##  left-truncation arms the observed entry times. Identical rows, identical
##  seed, identical hyperparameters throughout.
##
##  The standard arms do not depend on the entry vector, so in principle
##  they need fitting only once and can be carried into both phases
##  unchanged.
##
##  `noop` records how far apart each left-truncation arm sits from its
##  standard counterpart in phase 1. Both should be zero: the
##  counting-process Cox with entry 0 has the same risk sets as the
##  standard fit, and entry.time = 0 is documented to reproduce the
##  standard forest bit-for-bit. This has now been re-checked on every
##  replication of every simulation run to date (160 replications across
##  six different DGPs) -- Cox agrees to ~1e-14 (floating-point noise) and
##  RSF agrees exactly (0) every single time. With `verify_noop = FALSE`
##  (the default), the experiment trusts that guarantee instead of
##  re-proving it: the standard arms are not independently fitted at all,
##  their phase-1 (zero-entry) LT fit is reused in their place, and `noop`
##  is reported as NA. This cuts RSF fits per replication from 3 to 2 (and
##  Cox likewise), since the standard arm was the one fit that phase 1 and
##  phase 2 couldn't already share.
##
##  Set `verify_noop = TRUE` to restore the full independent check -- do
##  this after any change to the entry.time C code in the forked package,
##  since that is the one thing this guarantee actually depends on.

run_replication <- function(rep_id, setup, sim,
                            scorer = scorer_mad_vs_truth,
                            m_test = 200L,
                            verify_noop = FALSE) {
  seed_r <- setup$meta$master_seed + 1000L + rep_id
  set.seed(seed_r)

  tg   <- setup$time_grid
  arms <- build_arms(setup$rsf_hp)

  ## ---- one sample, shared by both phases ----
  ## Covariates are whatever is not part of the response, so a simulation
  ## can name them however it likes (x1..xp, or x1..x3 + z1..z2 to keep a
  ## nuisance/primary distinction). A name-prefix rule would silently drop
  ## any column that did not match.
  train <- sim$draw_train(setup)
  xn    <- setdiff(names(train), c("entry", "time", "status"))
  stopifnot(length(xn) > 0, all(c("entry", "time", "status") %in% names(train)))
  d     <- train[, c(xn, "time", "status")]      # entry withheld
  entry <- train$entry
  zero  <- rep(0, nrow(d))

  ## ---- test set and truth, shared by every arm ----
  Xtest <- sim$draw_test(setup, m_test)
  if (is.null(colnames(Xtest))) colnames(Xtest) <- xn
  ## Column order must line up with the training frame: the Cox arms index
  ## Xtest by coefficient name, and sim$truth reads columns positionally.
  stopifnot(identical(colnames(Xtest), xn))
  Xtest_df <- as.data.frame(Xtest)
  ctx <- list(Strue = sim$truth(Xtest, tg, setup))

  run_arm <- function(key, e) {
    a   <- arms[[key]]
    fit <- a$fit(d, e, xn, -seed_r)
    list(fit = fit, Shat = a$surv(fit, Xtest, Xtest_df, tg))
  }

  r_cox_p1  <- run_arm("cox_lt",  zero)
  r_rsf_p1  <- run_arm("rsf_lt",  zero)
  r_cox_p2  <- run_arm("cox_lt",  entry)
  r_rsf_p2  <- run_arm("rsf_lt",  entry)

  if (verify_noop) {
    r_cox_std <- run_arm("cox_std", zero)
    r_rsf_std <- run_arm("rsf_std", zero)
    noop <- c(cox = max(abs(r_cox_std$Shat - r_cox_p1$Shat)),
              rsf = max(abs(r_rsf_std$Shat - r_rsf_p1$Shat)))
  } else {
    r_cox_std <- r_cox_p1        # proven identical; see block comment above
    r_rsf_std <- r_rsf_p1
    noop <- c(cox = NA_real_, rsf = NA_real_)
  }

  S <- list(
    p1_cox_std = r_cox_std$Shat, p1_cox_lt = r_cox_p1$Shat,
    p1_rsf_std = r_rsf_std$Shat, p1_rsf_lt = r_rsf_p1$Shat,
    p2_cox_std = r_cox_std$Shat, p2_cox_lt = r_cox_p2$Shat,
    p2_rsf_std = r_rsf_std$Shat, p2_rsf_lt = r_rsf_p2$Shat
  )[ARM_KEYS]

  sc <- lapply(S, score_arm, ctx = ctx, scorer = scorer)

  ## ---- C-index: each arm's own-data survival curve, real outcomes ----
  Xtrain <- as.matrix(d[, xn, drop = FALSE])
  Shat_own <- list(
    p1_cox_std = cox_survival_matrix(r_cox_std$fit, Xtrain, tg),
    p1_cox_lt  = cox_survival_matrix(r_cox_p1$fit,  Xtrain, tg),
    p1_rsf_std = step_to_grid(r_rsf_std$fit$survival.oob, r_rsf_std$fit$time.interest, tg),
    p1_rsf_lt  = step_to_grid(r_rsf_p1$fit$survival.oob,  r_rsf_p1$fit$time.interest, tg),
    p2_cox_std = cox_survival_matrix(r_cox_std$fit, Xtrain, tg),  # phase-invariant, as above
    p2_cox_lt  = cox_survival_matrix(r_cox_p2$fit,  Xtrain, tg),
    p2_rsf_std = step_to_grid(r_rsf_std$fit$survival.oob, r_rsf_std$fit$time.interest, tg),
    p2_rsf_lt  = step_to_grid(r_rsf_p2$fit$survival.oob,  r_rsf_p2$fit$time.interest, tg)
  )[ARM_KEYS]
  cindex <- vapply(Shat_own, cindex_arm, numeric(1),
                   time = train$time, status = train$status, entry = entry)

  list(
    overall = vapply(sc, `[[`, numeric(1), "overall"),
    by_time = do.call(rbind, lapply(sc, `[[`, "by_time")),
    noop    = noop,
    cindex  = cindex
  )
}


## ---------------------------------------------------------------------
##  5. The replication loop
## ---------------------------------------------------------------------
##  Replications are independent and each seeds itself from rep_id, so
##  distributing them across cores gives the same answer as a serial run.
##  Workers source the engine and the simulation's DGP rather than
##  receiving exported closures, so a worker sees exactly what the parent
##  document sees. Each runs rfsrc single-threaded to avoid
##  oversubscription.
##
##  The cluster is respawned every batch rather than created once and
##  reused for the whole run. Timing logs from real knits showed the
##  final (often small) batch running 15-40x slower per replication than
##  every batch before it -- consistent with memory that a long-lived
##  worker accumulates across repeated forest fits not being handed back
##  to the OS, eventually forcing a swap. A fresh worker process starts
##  with a clean slate. Measured cost of one full respawn cycle
##  (stopCluster + makeCluster + re-source + reload packages, 9 workers):
##  about 1.1 sec -- negligible next to multi-minute batches.

spawn_cluster <- function(srcs, n_cores) {
  ## Namespace-qualified so the engine keeps working if `parallel` happens
  ## not to be attached — e.g. when knitr serves the sourcing chunk from
  ## cache, which restores that chunk's objects but not its library() calls.
  cl <- parallel::makeCluster(n_cores)
  parallel::clusterExport(cl, "srcs", envir = environment())
  parallel::clusterEvalQ(cl, {
    for (s in srcs) source(s)
    options(rf.cores = 1, mc.cores = 1)
    NULL
  })
  cl
}

run_experiment <- function(n_rep, setup, sim, sources,
                           scorer = scorer_mad_vs_truth,
                           m_test = 200L,
                           n_cores = max(1L, parallel::detectCores() - 1L),
                           verify_noop = FALSE) {
  srcs <- normalizePath(sources, mustWork = TRUE)

  batches <- split(seq_len(n_rep), ceiling(seq_len(n_rep) / n_cores))
  res <- vector("list", n_rep)
  t0  <- Sys.time()
  done <- 0L

  n_fits <- if (verify_noop) 6L else 4L
  cat(sprintf("running %d reps x %d fits on %d cores (respawned/batch)...\n",
              n_rep, n_fits, n_cores))
  for (b in batches) {
    cl <- spawn_cluster(srcs, n_cores)
    res[b] <- tryCatch(
      parallel::parLapply(cl, b, run_replication, setup = setup,
                          sim = sim, scorer = scorer, m_test = m_test,
                          verify_noop = verify_noop),
      finally = parallel::stopCluster(cl)
    )
    done <- done + length(b)
    el <- as.numeric(difftime(Sys.time(), t0, units = "mins"))
    cat(sprintf("  %3d/%d  (%.1f min elapsed, ~%.1f min left)\n",
                done, n_rep, el, el / done * (n_rep - done)))
  }

  by_time <- Reduce(`+`, lapply(res, `[[`, "by_time")) / n_rep
  rownames(by_time) <- ARM_KEYS

  list(
    overall      = as.data.frame(do.call(rbind, lapply(res, `[[`, "overall"))),
    by_time      = by_time,
    noop         = as.data.frame(do.call(rbind, lapply(res, `[[`, "noop"))),
    cindex       = as.data.frame(do.call(rbind, lapply(res, `[[`, "cindex"))),
    tgrid        = setup$time_grid,
    n_rep        = n_rep,
    m_test       = m_test,
    scorer_label = scorer$label,
    cindex_label = "C-index error (1-C, truncation-aware, lower=better) on training sample",
    sim_name     = sim$name
  )
}


## Tables and plots live in sim_report.R, which is intentionally excluded
## from the loop chunk's cache.extra hash so presentation can change
## without invalidating a completed run.
