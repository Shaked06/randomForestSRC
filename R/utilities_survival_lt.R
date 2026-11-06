## Shared entry-conditioned predictions and delayed-entry censoring.
## Upstream extracts grow/evaluation data separately; never replace validation
## outcomes with grow outcomes when computing an evaluation loss.
survival.score.setup.lt <- function(o, subset, cens.model, papply, entry.time,
                                     times = NULL) {
  if (!is.numeric(entry.time) || anyNA(entry.time) ||
      any(!is.finite(entry.time)) || any(entry.time < 0)) {
    stop("entry.time must contain finite, non-negative values")
  }
  dat <- .get.survival.performance.data(o, subset = subset)
  if (is.null(dat$eval.yvar)) {
    stop("evaluation outcomes are required to calculate Brier score or time-dependent AUC.")
  }
  if (is.null(dat$grow.yvar)) {
    stop("grow outcomes are required to estimate the censoring distribution.")
  }
  full.yvar <- .surv.get.element(dat$object, "yvar")
  if (length(entry.time) != nrow(full.yvar)) {
    stop("entry.time must have one value per evaluation observation before subsetting")
  }
  if (any(entry.time > full.yvar[, 1], na.rm = TRUE)) {
    stop("entry.time must not exceed observed follow-up for any evaluation subject")
  }
  time <- as.numeric(dat$eval.yvar[, 1])
  status <- as.numeric(dat$eval.yvar[, 2])
  grow.time <- as.numeric(dat$grow.yvar[, 1])
  grow.status <- as.numeric(dat$grow.yvar[, 2])
  if (any(!is.finite(c(time, grow.time))) ||
      anyNA(c(status, grow.status)) ||
      any(!c(status, grow.status) %in% c(0, 1))) {
    stop("Left-truncated survival scoring requires known follow-up times and binary event status")
  }
  entry <- entry.time[dat$subset]
  if (dat$object.type == "predict") {
    forest <- .surv.get.element(dat$object, "forest")
    grow.entry <- .surv.get.element(forest, "entry.time")
    if (is.null(grow.entry)) grow.entry <- rep(0, length(grow.time))
  } else {
    grow.entry <- entry.time
  }
  if (!is.numeric(grow.entry) || length(grow.entry) != length(grow.time) ||
      any(!is.finite(grow.entry)) || any(grow.entry < 0) ||
      any(grow.entry > grow.time)) {
    stop("Stored training entry times do not align with the censoring-model outcomes")
  }
  grid <- dat$time
  if (!is.null(times)) {
    grid <- sort(unique(as.numeric(times)))
    if (!length(grid) || any(!is.finite(grid))) {
      stop("times must contain finite evaluation times")
    }
  }
  if (any(dat$survival < 0 | dat$survival > 1, na.rm = TRUE)) {
    stop("Survival predictions must be between zero and one")
  }
  ## Condition on the original prediction grid, even when evaluation uses
  ## a coarser grid that omits a subject's entry time.
  survival.at.entry <- vapply(seq_along(entry), function(i) {
    k <- findInterval(entry[i], dat$time, left.open = TRUE)
    if (k == 0) 1 else dat$survival[i, k]
  }, numeric(1))
  if (any(is.finite(survival.at.entry) & survival.at.entry <= 0)) {
    stop("Survival at entry is zero; an entry-conditioned prediction is undefined")
  }
  raw.S <- t(.surv.step.matrix(dat$survival, dat$time, grid, initial = 1))
  S <- sweep(raw.S, 2, survival.at.entry, "/")
  eligible <- outer(grid, entry, ">=")
  if (any(S[eligible] > 1 + 1e-12, na.rm = TRUE)) {
    stop("Survival predictions must not increase after entry")
  }
  S[!eligible] <- NA_real_
  grow <- list(time = grow.time, status = grow.status, entry = grow.entry,
               xvar = dat$grow.xvar)
  evaluation <- list(time = time, status = status, entry = entry,
                     xvar = dat$eval.xvar)
  censoring <- survival.score.censoring.lt(grow, evaluation, cens.model, grid)
  list(event.info = .surv.event.info(dat$eval.yvar, grid),
       grow.event.info = .surv.event.info(dat$grow.yvar, grid),
       eval.event.info = .surv.event.info(dat$eval.yvar, grid),
       subset = dat$subset, mort = dat$mort, surv.ensb = raw.S,
       conditional.survival = S, entry.selected = entry,
       time.selected = time, status.selected = status, censoring = censoring)
}

survival.score.censoring.lt <- function(grow, evaluation, model, grid) {
  evaluate <- function(knots, survival, query, left = FALSE) {
    c(1, survival)[1L + findInterval(query, knots, left.open = left)]
  }
  n <- length(evaluation$time)
  if (!any(grow$status == 0)) {
    return(list(on.grid = if (model == "km") rep(1, length(grid)) else
                  matrix(1, length(grid), n),
                at.event = rep(1, n), at.entry = rep(1, n)))
  }
  if (model == "km") {
    knots <- sort(unique(grow$time[grow$status == 0]))
    hazard <- vapply(knots, function(tt) {
      risk <- sum(grow$entry <= tt & grow$time >= tt)
      ## Match upstream's convention: failures precede censoring at ties.
      after.failure <- risk - sum(grow$time == tt & grow$status == 1)
      sum(grow$time == tt & grow$status == 0) / after.failure
    }, numeric(1))
    G <- cumprod(1 - hazard)
    return(list(on.grid = evaluate(knots, G, grid),
                at.event = evaluate(knots, G, evaluation$time, left = TRUE),
                at.entry = evaluate(knots, G, evaluation$entry, left = TRUE)))
  }
  if (is.null(grow$xvar) || is.null(evaluation$xvar)) {
    stop("cens.model = \"rfsrc\" requires stored training and evaluation covariates")
  }
  x <- as.data.frame(grow$xvar)
  test.x <- as.data.frame(evaluation$xvar)
  internal <- tail(make.unique(c(names(x), ".rf.time", ".rf.cens", ".rf.entry")), 3)
  x[[internal[3]]] <- grow$entry
  test.x[[internal[3]]] <- evaluation$entry
  data <- x
  data[[internal[1]]] <- grow$time
  data[[internal[2]]] <- 1 - grow$status
  formula <- as.formula(paste0("Surv(", internal[1], ",", internal[2], ") ~ ."))
  fit <- rfsrc(formula, data, ntree = 50, nsplit = 1,
               splitrule = "random",
               nodesize = set.nodesize(length(grow$time), ncol(x)),
               entry.time = grow$entry, na.action = "na.impute", ntime = 0,
               perf.type = "none")
  prediction <- predict(fit, newdata = test.x,
                         na.action = "na.impute", perf.type = "none")
  G <- prediction$survival
  knots <- prediction$time.interest
  on.grid <- vapply(seq_len(n), function(i) {
    evaluate(knots, G[i, ], grid)
  }, numeric(length(grid)))
  on.grid <- matrix(on.grid, nrow = length(grid), ncol = n)
  list(on.grid = on.grid,
       at.event = vapply(seq_len(n), function(i) {
         evaluate(knots, G[i, ], evaluation$time[i], left = TRUE)
       }, numeric(1)),
       at.entry = vapply(seq_len(n), function(i) {
         evaluate(knots, G[i, ], evaluation$entry[i], left = TRUE)
       }, numeric(1)))
}
