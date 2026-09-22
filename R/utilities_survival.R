####################################################################
##
## survival functions
##
####################################################################
get.event.info <- function(obj, subset = NULL) {
  ## survival case
  if (grepl("surv", obj$family)) {
    if (!is.null(obj$yvar)) {
      if (is.null(subset)) {
        subset <- (1:nrow(cbind(obj$yvar)))
      }
        if (is.null(obj$subj)) { 
            r.dim <- 2
            time <- obj$yvar[subset, 1]
            cens <- obj$yvar[subset, 2]
        }
        else {
            r.dim <- 3
            start.time <- obj$yvar[subset, 1]
            time <- obj$yvar[subset, 2]
            cens <- obj$yvar[subset, 3]
        }
      ## censoring must be coded coherently
      if (!all(floor(cens) == abs(cens), na.rm = TRUE)) {
        stop("for survival families censoring variable must be coded as a non-negative integer")
      }
      ## Extract the unique event types.
      event <- na.omit(cens)[na.omit(cens) > 0]
      event.type <- sort(unique(event))
    }
    ##everything else
    else {
      r.dim <- 0
      event <- event.type <- cens <- cens <- time <- NULL
    }
    ## Set grid of time points.
    time.interest <- obj$time.interest
  }
  else {
    ## NULL for other families
    if ((obj$family == "regr+") | (obj$family == "class+")) {
      r.dim <- dim(obj$yvar)[2]
    }
    else {
      r.dim <- 1
    }
    event <- event.type <- cens <- time.interest <- cens <- time <- NULL
  }
  return(list(event = event, event.type = event.type, cens = cens,
              time.interest = time.interest, time = time, r.dim = r.dim))
}
get.grow.event.info <- function(yvar, fmly, need.deaths = TRUE, ntime = NULL) {
  if (grepl("surv", fmly)) {
    ##-----------------------------------------------------------
    ## survival, competing risks, or time dependent covariates
    ##-----------------------------------------------------------
    if (dim(yvar)[2] == 2) {
      ##---------------------------------
      ## survival or competing risks:
      ##---------------------------------
      r.dim <- 2
      time <- yvar[, 1]
      cens <- yvar[, 2]
      start.time <- NULL
      ## censoring must be coded coherently
      if (!all(floor(cens) == abs(cens), na.rm = TRUE)) {
        stop("for survival families censoring variable must be coded as a non-negative integer (perhaps the formula is set incorrectly?)")
      }
      ## check if deaths are available (if user specified)
      if (need.deaths && (all(na.omit(cens) == 0))) {
        stop("no deaths in data!")
      }
      ## Check for event time consistency.
      ## we over-ride this now to allow for negative time (see Stute)
      ##if (!all(na.omit(time) >= 0)) {
      ##  stop("time must be  positive")
      ##}
      ## Extract the unique event types.
      event.type <- unique(na.omit(cens))
      ## Ensure they are all greater than or equal to zero.
      if (sum(event.type >= 0) != length(event.type)) {
        stop("censoring variable must be coded as NA, 0, or greater than 0.")
      }
      ## Discard the censored state, if it exists.
      event <- na.omit(cens)[na.omit(cens) > 0]
      event.type <- unique(event)
      ## Set grid of time points.
      nonMissingOutcome <- which(!is.na(cens) & !is.na(time))
      nonMissingDeathFlag <- (cens[nonMissingOutcome] != 0)
      time.interest <- sort(unique(time[nonMissingOutcome[nonMissingDeathFlag]]))
      ## trim the time points if the user has requested it
      ## we also allow the user to pass requested time points
      if (!is.null(ntime) && !((length(ntime) == 1) && ntime == 0)) {
        if (length(ntime) == 1 && length(time.interest) > ntime) {
          time.interest <- time.interest[
            unique(round(seq.int(1, length(time.interest), length.out = ntime)))]
        }
        if (length(ntime) > 1) {
          time.interest <- unique(sapply(ntime, function(tt) {
            time.interest[max(1, sum(tt >= time.interest, na.rm = TRUE))]
          }))
        }
      }
    }
    ##-------------------------------
    ## time dependent covariates:
    ##-------------------------------
    else {
      r.dim <- 3
      start.time <- yvar[, 1]
      time <- yvar[, 2]
      cens <- yvar[, 3]
      ## censoring must be coded coherently
      if (!all(floor(cens) == abs(cens), na.rm = TRUE)) {
        stop("for survival families censoring variable must be coded as a non-negative integer (perhaps the formula is set incorrectly?)")
      }
      ## check if deaths are available (if user specified)
      if (need.deaths && (all(na.omit(cens) == 0))) {
        stop("no deaths in data!")
      }
      ## Check for event time consistency.
      if (!all(na.omit(time) >= 0)) {
        stop("time must be  positive")
      }
      ## Extract the unique event types.
      event.type <- unique(na.omit(cens))
      ## Ensure they are all greater than or equal to zero.
      if (sum(event.type >= 0) != length(event.type)) {
        stop("censoring variable must be coded as NA, 0, or greater than 0.")
      }
      ## Discard the censored state, if it exists.
      event <- na.omit(cens)[na.omit(cens) > 0]
      event.type <- unique(event)
      ## Set grid of time points.
      nonMissingOutcome <- which(!is.na(cens) & !is.na(time))
      nonMissingDeathFlag <- (cens[nonMissingOutcome] != 0)
      time.interest <- sort(unique(time[nonMissingOutcome[nonMissingDeathFlag]]))
      ## trim the time points if the user has requested it
      ## we also allow the user to pass requested time points
      if (!is.null(ntime) && !((length(ntime) == 1) && ntime == 0)) {
        if (length(ntime) == 1 && length(time.interest) > ntime) {
          ## select evenly spaced values over [0,1] and not event times 
          time.interest <- seq(0,  min(1, max(time[nonMissingOutcome])), length = ntime)
          time.interest <- time.interest[time.interest > 0]
        }
        if (length(ntime) > 1) {
          ## over-ride the default setting and allow the user to specify anything they want between [0,1]
          time.pt <- ntime <= min(1, max(time[nonMissingOutcome])) & ntime > 0
          if (sum(time.pt) == 0) {
            stop("the ntime vector supplied must be between [0,1]:", ntime)
          }
          time.interest <- sort(unique(ntime[time.pt]))
        }
      }
    }
  }
  ##---------------------
  ## other families
  ##---------------------
  else {
    if ((fmly == "regr+") | (fmly == "class+") | (fmly == "mix+")) {
      r.dim <- dim(yvar)[2]
    }
    else {
      if (fmly == "unsupv") {
        r.dim <- 0
      }
      else {
        r.dim <- 1
      }
    }
    event <- event.type <- cens <- time.interest <- cens <- time <- start.time <- NULL
  }
  return(list(event = event, event.type = event.type, cens = cens,
              time.interest = time.interest,
              time = time, start.time = start.time, r.dim = r.dim))
}
## ---------------------------------------------------------------------
##
## rmst 
##
## ---------------------------------------------------------------------
get.rmst <- function(o, tau.horizon = NULL, q = .95) {
  ## incoming parameter checks
  if (is.null(o)) {
    return(NULL)
  }
  if (o$family != "surv") {
    stop("this function only supports right-censored survival settings")
  }
  if (sum(inherits(o, c("rfsrc", "grow"), TRUE) == c(1, 2)) != 2 &
      sum(inherits(o, c("rfsrc", "predict"), TRUE) == c(1, 2)) != 2) {
    stop("This function only works for objects of class `(rfsrc, grow)' or '(rfsrc, predict)'")
  }
  ## extract time, survival (use OOB values if available)
  time <- o$time.interest
  if (is.null(o$survival.oob)) {
    surv <- o$survival.oob
  }
  else {
    surv <- o$survival
  }
  ## set the time horizon
  if (is.null(tau.horizon)) {
    ## can replace this with maximum
    ## tau.horizon <- max(time, na.rm = TRUE)
    tau.horizon <- quantile(time, probs = q, na.rm = TRUE)
  }
  ## adjustment for when time doesn't include tau.horizon
  etime <- sort(unique(c(time, tau.horizon)))
  surv <- cbind(1, surv)[, 1 + sIndex(time, etime)]
  time <- etime
  ## restrict time to tau horizon
  time.pt <- time <= tau.horizon
  ## calculate rmst for the restricted time
  c(surv[, time.pt, drop = FALSE] %*% diff(c(0, time[time.pt])))
}
## ---------------------------------------------------------------------
##
## brier score
##
## ---------------------------------------------------------------------
## trapezoidal rule
trapz <- function (x, y) {
  idx = 2:length(x)
  return(as.double((x[idx] - x[idx - 1]) %*% (y[idx] + y[idx - 1]))/2)
}
## returns an index of positions for evaluating a step function at selected times
sIndex <- function(x,y) {sapply(1:length(y), function(j) {sum(x <= y[j])})}
## set nodesize
set.nodesize <- function(n, p, nodesize = NULL) {
  if (is.null(nodesize)) {
    if (n <= 300 & p > n) {
      nodesize <- 2
    }
    else if (n <= 300 & p <= n) {
      nodesize <- 5
    }
    else if (n > 300 & n <= 2000) {
      nodesize <- 10
    }
    else {
      nodesize <- n / 200
    }
  }
  nodesize
}
## main brier function
## entry.time: optional left-truncation (entry/late-entry) time, one per
## TRAINING observation (same length and row order as the forest's own
## yvar -- not the `subset` being scored). NULL (the default) reproduces
## the standard Brier score exactly. See
## docs/brier_score_left_truncation.md for the three adjustments this
## implements: a per-time risk set restricted to subjects who had already
## entered, the predicted curve conditioned on having survived to entry,
## and a truncation weight alongside the existing censoring weight.
## ------------------------------------------------------------
##  Shared setup for the curve-based survival scorers
## ------------------------------------------------------------
##  get.brier.survival() and get.auc.survival() need exactly the same
##  ingredients: the fitted object's OOB (or in-bag) survival matrix mapped
##  onto the master time grid, the IPCW censoring distribution on that same
##  grid, and -- under left truncation -- the entry-time CDF G_L on that
##  grid. Factoring that block out is what keeps the two scorers from
##  drifting apart in how they build their weights.
##
##  `subset` arrives already resolved (NULL means "everything"), and
##  `cens.model` already match.arg'd by the caller.
survival.score.setup <- function(o, subset, cens.model, papply, entry.time) {
  if (o$family != "surv") {
    stop("this function only supports right-censored survival settings")
  }
  if (sum(inherits(o, c("rfsrc", "grow"), TRUE) == c(1, 2)) != 2 &
      sum(inherits(o, c("rfsrc", "forest"), TRUE) == c(1, 2)) != 2 &
      sum(inherits(o, c("rfsrc", "predict"), TRUE) == c(1, 2)) != 2) {
    stop("This function only works for objects of class `(rfsrc, grow)', '(rfsrc, forest)' or '(rfsrc, predict)'")
  }
  ## special handling if object is a forest
  if (sum(inherits(o, c("rfsrc", "forest"), TRUE) == c(1, 2)) == 2) {
    predO <- predict(o, perf.type = "none")
    o$predicted <- predO$predicted
    o$predicted.oob <- predO$predicted.oob
    o$survival.oob <- predO$survival.oob
    o$forest <- list()
    o$forest$yvar <- o$yvar
    o$forest$xvar <- o$xvar
  }
  ## use imputed missing time or censoring indicators
  if (!is.null(o$yvar) && !is.null(o$imputed.indv)) {
    o$yvar[o$imputed.indv, ] <- o$imputed.data[, 1:2]
  }
  ## subsetting: assumes entire data set to be used if not specified
  if (is.null(subset)) {
    subset <- 1:length(o$predicted)
  }
  else {
    ## convert the user specified subset into a usable form
    if (is.logical(subset)) subset <- which(subset)
    subset <- unique(subset[subset >= 1 & subset <= length(o$predicted)])
    if (length(subset) == 0) {
      stop("'subset' not set properly.")
    }
  }
  ## yvar is used for building the training (grow) censoring distribution
  ## however, there is no guarantee that yvar will exist in predict mode
  ## the forest however always contains yvar, so we use that
  ## also see above for special handling of forest
  pred.no.y <- is.null(o$yvar)
  yvar <- o$forest$yvar
  o$yvar <- yvar
  if (!is.null(entry.time)) {
    if (length(entry.time) != nrow(yvar)) {
      stop("entry.time must have one value per training observation")
    }
    if (any(is.na(entry.time)) || any(entry.time < 0)) {
      stop("entry.time must be non-negative and non-missing")
    }
    if (any(entry.time > yvar[, 1], na.rm = TRUE)) {
      stop("entry.time must not exceed the observed follow-up time for any subject")
    }
  }
  event.info <- get.event.info(o)
  ## obtain subset event info, but then put original yvar back
  if (!pred.no.y) {
    o$yvar <- yvar[subset,, drop = FALSE]
    subset.event.info <- get.event.info(o)
    o$yvar <- yvar
  }
  else {
    subset.event.info <- NULL
  }
  ## use OOB values if available
  if (is.null(o$predicted.oob)) {
    mort <- o$predicted[subset]
    surv.ensb <- t(o$survival[subset,, drop = FALSE])
  }
  else {
    mort <- o$predicted.oob[subset]
    surv.ensb <- t(o$survival.oob[subset,, drop = FALSE])
  }
  ##-------------------------------------------------------------------------------
  ##
  ## censoring distribution estimator for training (grow) data
  ## match time to grow master list, time.interest
  ##
  ##-------------------------------------------------------------------------------
  ## we match the censoring times with the master list time.interest
  ## this unifies all further calculations
  censTime <- sort(unique(event.info$time[event.info$cens == 0]))
  censTime.pt <- c(sIndex(censTime, event.info$time.interest))
  ## check to see if there are censoring cases
  if (length(censTime) > 0) {
    ## KM censoring distribution estimator
    if (cens.model == "km") {
      censModel.obj <- do.call(rbind, papply(1:length(censTime), function(j) {
        c(sum(event.info$time >= censTime[j], na.rm = TRUE),
          sum(event.info$time[event.info$cens == 0] == censTime[j], na.rm = TRUE))
      }))
      Y <- censModel.obj[, 1]
      d <- censModel.obj[, 2]
      r <- d / (Y + 1 * (Y == 0))
      cens.dist <- c(1, exp(-cumsum(r)))[1 + censTime.pt]
    }
    ## rfsrc censoring distribution estimator using random splitting
    else {
      cens.dta <- data.frame(time = o$forest$yvar[, 1],
                             cens = 1 * (o$forest$yvar[, 2] == 0),
                             o$forest$xvar)
      cens.o <- rfsrc(Surv(time, cens) ~ ., cens.dta,
                      ntree = 50,
                      nsplit = 1,
                      splitrule = "random",
                      nodesize = set.nodesize(nrow(cens.dta), ncol(o$forest$xvar)),
                      perf.type = "none")
      if (!is.null(o$imputed.indv)) {
        o$xvar[o$imputed.indv, ] <- o$imputed.data[, -(1:2)]
      }
      cens.dist <- predict(cens.o, o$xvar[subset,, drop = FALSE])$survival
      censTime.pt <- c(sIndex(cens.o$time.interest, event.info$time.interest))
      cens.dist <- t(cbind(1, cens.dist)[, 1 + censTime.pt])
    }
  }
  ## no censoring cases; assign a default distribution
  else {
    cens.dist <- rep(1, length(censTime.pt))
  }
  ## left truncation: empirical CDF of entry times, on the master grid.
  ## Unlike the censoring distribution, entry times are directly observed
  ## for every retained subject (no further censoring on entry itself), so
  ## a plain empirical CDF is the right estimator here -- no KM/risk-set
  ## machinery needed, unlike cens.dist above.
  if (!is.null(entry.time)) {
    GL.vec <- vapply(event.info$time.interest, function(tt) mean(entry.time <= tt), numeric(1))
  }
  else {
    GL.vec <- NULL
  }
  list(o = o,
       subset = subset,
       pred.no.y = pred.no.y,
       yvar = yvar,
       event.info = event.info,
       subset.event.info = subset.event.info,
       mort = mort,
       surv.ensb = surv.ensb,
       cens.dist = cens.dist,
       GL = GL.vec)
}
get.brier.survival <- function(o, subset, cens.model = c("km", "rfsrc"), papply = lapply,
                               entry.time = NULL) {
  ## incoming parameter checks
  if (is.null(o)) {
    return(NULL)
  }
  ## verify the cens.model option
  cens.model <- match.arg(cens.model, c("km", "rfsrc"))
  ## everything from here down to the KM block is shared with
  ## get.auc.survival() -- see survival.score.setup() above.
  setup <- survival.score.setup(o, if (missing(subset)) NULL else subset,
                                cens.model, papply, entry.time)
  o                 <- setup$o
  subset            <- setup$subset
  pred.no.y         <- setup$pred.no.y
  yvar              <- setup$yvar
  event.info        <- setup$event.info
  subset.event.info <- setup$subset.event.info
  mort              <- setup$mort
  surv.ensb         <- setup$surv.ensb
  cens.dist         <- setup$cens.dist
  GL.vec            <- setup$GL
  ##-------------------------------------------------------------------------------
  ##
  ## KM for training/testing data - for testing, there must be y
  ## match time to grow master list, time.interest
  ##
  ##-------------------------------------------------------------------------------
  if (!pred.no.y) {
    km.obj <- do.call(rbind, papply(1:length(subset.event.info$time.interest), function(j) {
      c(sum(subset.event.info$time >= subset.event.info$time.interest[j], na.rm = TRUE),
        sum(subset.event.info$time[subset.event.info$cens != 0] == subset.event.info$time.interest[j], na.rm = TRUE))
    }))
    Y <- km.obj[, 1]
    d <- km.obj[, 2]
    r <- d / (Y + 1 * (Y == 0))
    surv.aalen <- exp(-cumsum(r))[1 + sIndex(subset.event.info$time.interest, event.info$time.interest)]
  }
  else {
    surv.aalen <- NULL
  }
  ## brier calculations
  brier.matx <- do.call(rbind, papply(1:ncol(surv.ensb), function(i) {
    tau <-  event.info$time
    event <- event.info$cens
    t.unq <- event.info$time.interest
    cens.pt <- sIndex(t.unq, tau[i])
    if (cens.model == "km") {
      c1 <- 1 * (tau[i] <= t.unq & event[i] != 0)/c(1, cens.dist)[1 + cens.pt]
      c2 <- 1 * (tau[i] > t.unq) / cens.dist
    }
    else {
      c1 <- 1 * (tau[i] <= t.unq & event[i] != 0)/c(1, cens.dist[, i])[1 + cens.pt]
      c2 <- 1 * (tau[i] > t.unq) / cens.dist[, i]
    }
    surv.i <- surv.ensb[, i]
    not.in.risk.set <- FALSE
    ## left truncation adjustments -- see docs/brier_score_left_truncation.md.
    ## entry.time indexed the same way as tau/event above (by i, not by the
    ## `subset` argument): entry.time is supplied at the scale of the full
    ## training sample, matching how tau/event are read from event.info,
    ## which is itself unsubsetted (see comment above on subset.event.info).
    if (!is.null(entry.time)) {
      entry.i <- entry.time[i]
      ## Adjustment 2: condition the predicted curve on having survived to
      ## this subject's own entry time -- S(t|X) / S(entry|X).
      S.entry.i <- c(1, surv.i)[1 + sIndex(t.unq, entry.i)]
      surv.i <- surv.i / S.entry.i
      ## Weighting: truncation-weight the same way c1/c2 already
      ## censoring-weight, using G_L at the same two time arguments
      ## (G_L(T_i^-) for c1, G_L(t) for c2). gl1 is computed directly
      ## rather than by reusing cens.pt's grid-index trick: that trick
      ## relies on the prepended value being exactly correct for any time
      ## before the first grid point, which holds for cens.dist (survival
      ## is exactly 1 at time 0) but not for G_L -- entries can easily
      ## have already happened before the first *death* time in the grid,
      ## so a prepended 0 is not a safe assumption here. Division is
      ## always safe on cells that survive the risk-set filter below: if
      ## this subject is in the risk set at t (entry.i <= t), it
      ## contributes to G_L at t itself, so G_L cannot be zero there.
      gl1 <- mean(entry.time < tau[i])
      gl2 <- GL.vec
      c1 <- c1 / gl1
      c2 <- c2 / gl2
      ## Adjustment 1: risk set R_L(t) = {i : entry_i <= t} -- exclude time
      ## points this subject hadn't entered by yet. colMeans(na.rm=TRUE)
      ## below then averages only over subjects actually in R_L(t),
      ## exactly matching the spec's 1/|R_L(t)| normalization.
      not.in.risk.set <- t.unq < entry.i
    }
    out <- (1 * (tau[i] > t.unq) - surv.i)^2 * (c1 + c2)
    out[not.in.risk.set] <- NA_real_
    out
  }))
  brier.score <- data.frame(time = event.info$time.interest,
                            brier.score = colMeans(brier.matx, na.rm = TRUE))
  ## crps - continuous rank probability score
  crps <- trapz(brier.score$time, brier.score$brier.score)
  ## return the goodies
  list(brier.matx = brier.matx,
       brier.score = brier.score,
       cens.dist = cens.dist,
       crps = crps,
       crps.std = crps / max(brier.score$time),
       time = event.info$time.interest,
       event.info = event.info,
       subset = subset,
       mort = mort,
       surv.aalen = surv.aalen,
       surv.ensb = surv.ensb,
       GL = if (is.null(entry.time)) NULL else GL.vec)
}
## ---------------------------------------------------------------------
##  Time-dependent (cumulative/dynamic) AUC -- see docs/auc_left_truncation.md
## ---------------------------------------------------------------------
##  Sits between the two metrics already here. Like get.cindex() it is
##  rank-based -- it never asks whether a predicted probability is
##  numerically right, only whether the ordering is right. Like
##  get.brier.survival() it is evaluated at a fixed time t against the risk
##  set alive at t, which is precisely where left truncation does damage.
##
##  NOT to be confused with the exported get.auc(), which is the Hand & Till
##  multiclass *classification* AUC and has nothing to do with survival.
##
##  At each grid time t the sample splits into cases (T_i <= t, delta_i = 1)
##  and controls (T_j > t); each subject carries the marker
##  M_i(t) = 1 - S(t|X_i), i.e. predicted risk of the event by t. The AUC is
##  the IPCW-weighted probability that a case outranks a control.
##
##  Note this is HIGHER-is-better, unlike MAD, the Brier score, and
##  get.cindex()'s error-rate convention. Callers wanting a common direction
##  should use 1 - auc.
##
##  entry.time = NULL (the default) gives the ordinary right-censored
##  estimator; supplying it applies all three left-truncation adjustments
##  from the spec.
get.auc.survival <- function(o, subset, cens.model = c("km", "rfsrc"), papply = lapply,
                             entry.time = NULL) {
  ## incoming parameter checks
  if (is.null(o)) {
    return(NULL)
  }
  ## verify the cens.model option
  cens.model <- match.arg(cens.model, c("km", "rfsrc"))
  setup <- survival.score.setup(o, if (missing(subset)) NULL else subset,
                                cens.model, papply, entry.time)
  subset     <- setup$subset
  event.info <- setup$event.info
  mort       <- setup$mort
  surv.ensb  <- setup$surv.ensb      ## grid x subject, subject in `subset` order
  cens.dist  <- setup$cens.dist
  GL.vec     <- setup$GL
  t.unq <- event.info$time.interest
  n <- ncol(surv.ensb)
  ## Subject-level outcomes, aligned to the COLUMNS of surv.ensb. event.info
  ## is built from the full training yvar, while surv.ensb holds only the
  ## `subset` rows, so the two must be brought onto a common index before
  ## they can be paired. (get.brier.survival() pairs them without this step;
  ## harmless on the default subset = everything, which is all the
  ## simulations use, but not in general.)
  tau     <- event.info$time[subset]
  event   <- event.info$cens[subset]
  entry.s <- if (is.null(entry.time)) NULL else entry.time[subset]
  ##-------------------------------------------------------------------------------
  ##
  ## subject-level quantities that do not depend on the evaluation time
  ##
  ##-------------------------------------------------------------------------------
  ## grid index of each subject's own observed time, for reading G_C(T_i^-)
  cens.pt <- sIndex(t.unq, tau)
  if (cens.model == "km") {
    gc.case <- c(1, cens.dist)[1 + cens.pt]
  }
  else {
    gc.case <- vapply(1:n, function(i) c(1, cens.dist[, i])[1 + cens.pt[i]], numeric(1))
  }
  w.case <- 1 / gc.case
  if (!is.null(entry.s)) {
    ## Adjustment 3 (case half): truncation weight 1/G_L(T_i^-). Computed
    ## directly rather than by the grid-index trick used for the censoring
    ## weight -- that trick relies on a prepended value being exactly right
    ## before the first grid point, which holds for a survival function
    ## (exactly 1 at time 0) but not for G_L, since entries routinely
    ## happen before the first death time in the grid.
    ## The marginal G_L is estimated from the FULL entry vector, not the
    ## subset: it is a property of the study's entry process, not of
    ## whichever subjects are being scored.
    gl.case <- vapply(tau, function(ti) mean(entry.time < ti), numeric(1))
    w.case <- w.case / gl.case
    ## Adjustment 2: condition each predicted curve on having survived to
    ## that subject's own entry time -- S(t|X)/S(entry|X). This genuinely
    ## reorders subjects rather than rescaling them all alike (the divisor
    ## is subject-specific), so a rank-based metric cannot skip it.
    S.entry <- vapply(1:n, function(i) {
      c(1, surv.ensb[, i])[1 + sIndex(t.unq, entry.s[i])]
    }, numeric(1))
    surv.ensb <- sweep(surv.ensb, 2, S.entry, "/")
  }
  ##-------------------------------------------------------------------------------
  ##
  ## AUC at each evaluation time
  ##
  ##-------------------------------------------------------------------------------
  auc.vec <- unlist(papply(1:length(t.unq), function(k) {
    tk <- t.unq[k]
    ## marker = predicted risk of the event by t. Ranking on 1 - S is the
    ## same as ranking on -S, but the explicit form keeps "higher marker =
    ## higher risk = should be the case" readable.
    marker <- 1 - surv.ensb[k, ]
    is.case <- (tau <= tk) & (event != 0)
    is.ctrl <- (tau > tk)
    if (!is.null(entry.s)) {
      ## Adjustment 1: risk set R_L(t) = {i : entry_i <= t}. For a case this
      ## is automatic (entry_i <= T_i <= t), so in practice this only ever
      ## drops controls who had not yet entered at t -- which is exactly the
      ## group an unadjusted AUC wrongly credits the model for ranking
      ## "correctly" as event-free.
      in.risk.set <- entry.s <= tk
      is.case <- is.case & in.risk.set
      is.ctrl <- is.ctrl & in.risk.set
    }
    ## control weight 1/(G_C(t) G_L(t)), read at the evaluation time
    gc.ctrl <- if (cens.model == "km") cens.dist[k] else cens.dist[k, ]
    w.ctrl <- rep_len(1 / gc.ctrl, n)
    if (!is.null(entry.s)) {
      w.ctrl <- w.ctrl / GL.vec[k]
    }
    ## a subject with a non-finite weight (zero estimated G_C or G_L) carries
    ## no usable information at this time point
    is.case <- is.case & is.finite(w.case) & is.finite(marker)
    is.ctrl <- is.ctrl & is.finite(w.ctrl) & is.finite(marker)
    if (!any(is.case) || !any(is.ctrl)) {
      return(NA_real_)
    }
    mi <- marker[is.case]; wi <- w.case[is.case]
    mj <- marker[is.ctrl]; wj <- w.ctrl[is.ctrl]
    ## Weighted Mann-Whitney numerator, done by sorting rather than by an
    ## outer product: the case x control matrix is O(n^2) per time point and
    ## there is one time point per death, which is quadratic-in-n work
    ## repeated n times on realistic sample sizes.
    ord <- order(mj)
    mj.s <- mj[ord]
    ## cw[1 + m] = total control weight among the m smallest markers
    cw <- c(0, cumsum(wj[ord]))
    ## findInterval counts controls with marker <= mi; left.open = TRUE
    ## counts those strictly below. The gap between them is the tie mass,
    ## which gets half credit. Ties are not a corner case here: subjects
    ## sharing terminal nodes across every tree get identical curves.
    W.le <- cw[1 + findInterval(mi, mj.s)]
    W.lt <- cw[1 + findInterval(mi, mj.s, left.open = TRUE)]
    num <- sum(wi * (W.lt + 0.5 * (W.le - W.lt)))
    den <- sum(wi) * sum(wj)
    if (den <= 0) NA_real_ else num / den
  }))
  auc <- data.frame(time = t.unq, auc = auc.vec)
  ## Headline figure: time-averaged AUC over the range where it is defined.
  ## Restricting the integral to the non-NA points (rather than dropping
  ## them and integrating across the hole) keeps the summary number and the
  ## plotted curve from disagreeing, same contract as crps.std.
  ok <- !is.na(auc.vec)
  iauc <- if (sum(ok) >= 2 && diff(range(t.unq[ok])) > 0) {
    trapz(t.unq[ok], auc.vec[ok]) / diff(range(t.unq[ok]))
  }
  else {
    NA_real_
  }
  ## return the goodies
  list(auc = auc,
       iauc = iauc,
       cens.dist = cens.dist,
       time = t.unq,
       event.info = event.info,
       subset = subset,
       mort = mort,
       surv.ensb = surv.ensb,
       GL = GL.vec)
}
## ------------------------------------------------------------
## Uno weights
## - training mode: KM or OOB KM
## - test mode: works generically
## - censors happen after deaths at tied times
## ------------------------------------------------------------
## fit censoring KM on training outcomes only
km_censor_fit <- function(time, status) {
  stopifnot(length(time) == length(status))
  ok <- !is.na(time) & !is.na(status)
  time   <- as.numeric(time[ok])
  status <- as.integer(status[ok])
  n <- length(time)
  if (n == 0L) stop("No non-missing training outcomes.")
  ord <- order(time)
  t <- time[ord]
  s <- status[ord]
  times <- numeric(n)
  G     <- numeric(n)
  surv   <- 1.0
  n_risk <- n
  k <- 0L
  i <- 1L
  while (i <= n) {
    ti <- t[i]
    j <- i
    d_death <- 0L
    d_cens  <- 0L
    while (j <= n && t[j] == ti) {
      if (s[j] == 1L) d_death <- d_death + 1L else d_cens <- d_cens + 1L
      j <- j + 1L
    }
    ## Update censoring survival AFTER removing deaths at this time
    n_after_death <- n_risk - d_death
    if (d_cens > 0L) {
      if (n_after_death <= 0L) {
        surv <- 0.0
      } else {
        surv <- surv * (1 - d_cens / n_after_death)
      }
    }
    k <- k + 1L
    times[k] <- ti
    G[k]     <- surv
    n_risk <- n_risk - (d_death + d_cens)
    i <- j
  }
  list(time = times[1L:k], G = G[1L:k])
}
## Generic left-limit step evaluation: returns Ghat(t-)
## for arbitrary t_new using knots/time_knots and post-step values G.
uno_Ghat_minus_predict <- function(time_knots, G, t_new) {
  t_new <- as.numeric(t_new)
  out <- rep(NA_real_, length(t_new))
  ok <- !is.na(t_new)
  if (!any(ok)) return(out)
  if (length(time_knots) == 0L) {
    out[ok] <- 1.0
    return(out)
  }
  idx <- findInterval(t_new[ok], time_knots, left.open = TRUE)
  out[ok] <- ifelse(idx == 0L, 1.0, G[idx])
  out
}
## Effective sample size of positive weights
uno_ess <- function(w) {
  w <- w[is.finite(w) & !is.na(w) & (w > 0)]
  if (length(w) == 0L) return(NA_real_)
  (sum(w)^2) / sum(w^2)
}
## Choose gmin automatically from training event-time Ghat(t-).
##
## Input:  G_event = vector of Ghat(t-) evaluated at event times only.
## Output: list(gmin, ess_target, ess_kept, n_events, n_dropped, wmax_kept)
##
## Rule: drop the largest weights (smallest G) until ESS >= ess_target,
## where ess_target = max(ess_min, ceil(ess_frac * n_events)).
uno_choose_gmin_auto <- function(G_event,
                                 eps = 1e-12,
                                 ess_frac = 0.20,
                                 ess_min  = 20L) {
  g <- as.numeric(G_event)
  g <- g[is.finite(g) & !is.na(g)]
  d <- length(g)
  if (d <= 1L) {
    return(list(gmin = 0.0, ess_target = NA_real_, ess_kept = NA_real_,
                n_events = d, n_dropped = 0L, wmax_kept = NA_real_))
  }
  ## If essentially no censoring (G ~ 1), no trimming needed
  if (min(g) >= 1 - 1e-12) {
    return(list(gmin = 0.0, ess_target = d, ess_kept = d,
                n_events = d, n_dropped = 0L, wmax_kept = 1.0))
  }
  ## weights are monotone in g: smaller g => larger weight
  g_sorted <- sort(g)                       # ascending g
  w_desc   <- 1.0 / pmax(g_sorted, eps)^2   # descending weights
  ## prefix sums (with leading 0)
  p1 <- c(0.0, cumsum(w_desc))
  p2 <- c(0.0, cumsum(w_desc * w_desc))
  ess_target <- max(as.integer(ess_min), as.integer(ceiling(ess_frac * d)))
  ess_target <- min(ess_target, d)
  ## drop k largest weights; must keep at least ess_target events
  best_k <- 0L
  best_ess <- NA_real_
  for (k in 0L:(d - ess_target)) {
    sum_w  <- p1[d + 1L] - p1[k + 1L]
    sum_w2 <- p2[d + 1L] - p2[k + 1L]
    ess_k  <- if (sum_w2 > 0) (sum_w * sum_w) / sum_w2 else NA_real_
    if (is.finite(ess_k) && (ess_k >= ess_target)) {
      best_k <- k
      best_ess <- ess_k
      break
    }
  }
  ## If never hit the target (rare), keep only ess_target events
  if (!is.finite(best_ess)) {
    best_k <- d - ess_target
    sum_w  <- p1[d + 1L] - p1[best_k + 1L]
    sum_w2 <- p2[d + 1L] - p2[best_k + 1L]
    best_ess <- if (sum_w2 > 0) (sum_w * sum_w) / sum_w2 else NA_real_
  }
  gmin <- g_sorted[best_k + 1L]
  wmax <- 1.0 / pmax(gmin, eps)^2
  list(gmin = gmin,
       ess_target = ess_target,
       ess_kept = best_ess,
       n_events = d,
       n_dropped = best_k,
       wmax_kept = wmax)
}
## Train-mode Uno weights
get.uno.weights.train <- function(time, status,
                                  gmin = "auto",
                                  ess_frac = 0.20,
                                  ess_min  = 20L,
                                  eps = 1e-12,
                                  eps_keep = .Machine$double.eps,
                                  drop_if_G0 = FALSE,
                                  return_fit = TRUE) {
  stopifnot(length(time) == length(status))
  ## Fit KM censoring curve on training outcomes
  fit <- km_censor_fit(time, status)
  ## Global Ghat(t-) used for gating and (also) weight magnitude
  G_gate <- uno_Ghat_minus_predict(fit$time, fit$G, time)
  ## Decide gmin (train-once)
  if (is.character(gmin)) {
    gmin <- match.arg(gmin, c("auto", "none"))
    if (gmin == "none") {
      gmin_used <- 0.0
      ginfo <- list(gmin = 0.0)
    } else {
      ## events = non-censored cases (otherwise breaks for CR)
      ev <- !is.na(status) & (as.integer(status) != 0L) & !is.na(G_gate)
      ginfo <- uno_choose_gmin_auto(G_gate[ev], eps = eps,
                                    ess_frac = ess_frac, ess_min = ess_min)
      gmin_used <- ginfo$gmin
    }
  } else {
    gmin_used <- as.numeric(gmin)[1L]
    if (!is.finite(gmin_used) || gmin_used < 0) gmin_used <- 0.0
    ginfo <- list(gmin = gmin_used)
  }
  ## Missing => exclude (weight 0)
  miss <- is.na(time) | is.na(status)
  G_gate[miss] <- NA_real_
  w <- rep(0.0, length(time))
  ok <- !is.na(G_gate)
  if (!drop_if_G0) {
    ## keep-as-comparator always; event contribution trimmed by gmin
    keep <- ok & (G_gate >= gmin_used)
    drop <- ok & !keep
    if (any(keep)) {
      Gsafe <- pmax(G_gate[keep], eps)
      w[keep] <- 1.0 / (Gsafe * Gsafe)
    }
    if (any(drop)) {
      w[drop] <- eps_keep
    }
  } else {
    ## trim additionally when G is essentially 0
    keep <- ok & (G_gate >= gmin_used) & (G_gate > eps)
    drop <- ok & !keep
    if (any(keep)) {
      Gsafe <- pmax(G_gate[keep], eps)
      w[keep] <- 1.0 / (Gsafe * Gsafe)
    }
    if (any(drop)) {
      w[drop] <- eps_keep
    }
  }
  ## Store once; test will reuse automatically
  fit$gmin      <- gmin_used
  fit$eps_keep  <- eps_keep
  fit$gmin_info <- ginfo
  if (return_fit) {
    return(list(weight = w,
                Ghat_minus = G_gate,
                fit = fit))
  }
  w
}
## Test-mode Uno weights
get.uno.weights.test <- function(time_test, fit,
                                 eps = 1e-12,
                                 drop_if_G0 = FALSE) {
  if (is.null(fit$time) || is.null(fit$G))
    stop("fit must be a list with elements $time and $G")
  gmin_used <- if (!is.null(fit$gmin) && is.finite(fit$gmin)) fit$gmin else 0.0
  eps_keep  <- if (!is.null(fit$eps_keep) && is.finite(fit$eps_keep)) fit$eps_keep else .Machine$double.eps
  G_gate <- uno_Ghat_minus_predict(fit$time, fit$G, time_test)
  w <- rep(0.0, length(time_test))
  ok <- !is.na(G_gate)
  if (!drop_if_G0) {
    keep <- ok & (G_gate >= gmin_used)
  } else {
    keep <- ok & (G_gate >= gmin_used) & (G_gate > eps)
  }
  drop <- ok & !keep
  if (any(keep)) {
    Gsafe <- pmax(G_gate[keep], eps)
    w[keep] <- 1.0 / (Gsafe * Gsafe)
  }
  if (any(drop)) {
    w[drop] <- eps_keep
  }
  w
}
## ------------------------------------------------------------
## Convenience helper for test evaluation
## ------------------------------------------------------------
uno.prepare.test <- function(time_test, status_test, fit,
                             eps = 1e-12, drop_if_G0 = FALSE) {
  w <- get.uno.weights.test(time_test, fit, eps = eps, drop_if_G0 = drop_if_G0)
  list(time = time_test, status = status_test, weight = w)
}
## ------------------------------------------------------------
## Convenience one-liner: training weights only
## ------------------------------------------------------------
get.uno.weights <- function(time, status) {
  get.uno.weights.train(time, status, return_fit = FALSE)
}
