## Practical entry-restricted IPCW score from the project outline, section 9.2.
## See docs/brier_score_left_truncation.md for the conditioning and assumptions.
get.brier.survival.lt <- function(o, subset, cens.model, papply, entry.time) {
  if (!is.numeric(entry.time) || anyNA(entry.time) ||
      any(!is.finite(entry.time)) || any(entry.time < 0)) {
    stop("entry.time must contain finite, non-negative values")
  }
  setup <- survival.score.setup(o, subset, cens.model, papply, entry.time,
                                 compute.censoring = FALSE)
  info <- setup$event.info
  time <- info$time
  status <- info$cens
  grid <- info$time.interest
  selected <- setup$subset
  raw.S <- setup$surv.ensb
  if (any(!is.finite(time)) || anyNA(status) || any(!status %in% c(0, 1))) {
    stop("Left-truncated Brier scoring requires known follow-up times and binary event status")
  }
  if (any(raw.S < 0 | raw.S > 1, na.rm = TRUE)) {
    stop("Survival predictions must be between zero and one")
  }
  entry <- entry.time[selected]
  ## The forest predicts from time zero. Conditioning on inclusive entry
  ## T >= L uses S(L^-), so failures exactly at entry remain possible.
  survival.at.entry <- vapply(seq_along(selected), function(i) {
    k <- findInterval(entry[i], grid, left.open = TRUE)
    if (k == 0) 1 else raw.S[k, i]
  }, numeric(1))
  if (any(is.finite(survival.at.entry) & survival.at.entry <= 0)) {
    stop("Survival at entry is zero; an entry-conditioned prediction is undefined")
  }
  S <- sweep(raw.S, 2, survival.at.entry, "/")
  eligible <- outer(grid, entry, ">=")
  if (any(S[eligible] > 1 + 1e-12, na.rm = TRUE)) {
    stop("Survival predictions must not increase after entry")
  }
  S[!eligible] <- NA_real_
  censoring <- brier.lt.censoring(setup$o, time, status, entry.time, selected,
                                  cens.model, grid)
  tau <- time[selected]
  event <- status[selected]
  contributions <- do.call(rbind, papply(seq_along(selected), function(i) {
    out <- rep(NA_real_, length(grid))
    eligible <- entry[i] <= grid & is.finite(S[, i])
    out[eligible] <- 0
    failed <- eligible & tau[i] <= grid & event[i] == 1
    alive <- eligible & tau[i] > grid
    if (any(failed) || any(alive)) {
      g.entry <- censoring$at.entry[i]
      g.event <- censoring$at.event[i]
      g.time <- if (cens.model == "km") censoring$on.grid else censoring$on.grid[, i]
      if (g.entry <= 0 || !is.finite(g.entry) ||
          (any(failed) && (g.event <= 0 || !is.finite(g.event))) ||
          any(g.time[alive] <= 0 | !is.finite(g.time[alive]))) {
        stop("Censoring survival is zero or undefined where the Brier score needs it; restrict the evaluation horizon")
      }
      out[failed] <- S[failed, i]^2 * g.entry / g.event
      out[alive] <- (1 - S[alive, i])^2 * g.entry / g.time[alive]
    }
    out
  }))
  score <- colMeans(contributions, na.rm = TRUE)
  score[is.nan(score)] <- NA_real_
  crps <- if (anyNA(score)) NA_real_ else trapz(grid, score)
  list(brier.matx = contributions,
       brier.score = data.frame(time = grid, brier.score = score),
       cens.dist = censoring$on.grid, crps = crps,
       crps.std = if (max(grid) > 0) crps / max(grid) else NA_real_,
       time = grid, event.info = info,
       subset = selected, mort = setup$mort, surv.aalen = NULL,
       surv.ensb = raw.S, conditional.survival = S, GL = NULL,
       score.type = "entry-restricted IPCW Brier",
       censoring.at.entry = censoring$at.entry,
       n.eligible = colSums(is.finite(contributions)))
}

brier.lt.censoring <- function(o, time, status, entry, selected, model, grid) {
  ## Events use the left limit G(U^-); controls use G(t). Entry is inclusive,
  ## so the conditional observation probability starts at G(L^-).
  evaluate <- function(knots, survival, query, left = FALSE) {
    c(1, survival)[1L + findInterval(query, knots, left.open = left)]
  }
  if (!any(status == 0)) {
    return(list(on.grid = if (model == "km") rep(1, length(grid)) else
                  matrix(1, length(grid), length(selected)),
                at.event = rep(1, length(selected)),
                at.entry = rep(1, length(selected))))
  }
  if (model == "km") {
    knots <- sort(unique(time[status == 0]))
    hazard <- vapply(knots, function(tt) {
      ## Retain the ordinary scorer's inclusive event-time convention.
      risk <- sum(entry <= tt & time >= tt)
      sum(time == tt & status == 0) / risk
    }, numeric(1))
    ## Preserve the ordinary scorer's exp(-Nelson--Aalen) estimator.
    G <- exp(-cumsum(hazard))
    return(list(on.grid = evaluate(knots, G, grid),
                at.event = evaluate(knots, G, time[selected], left = TRUE),
                at.entry = evaluate(knots, G, entry[selected], left = TRUE)))
  }
  ## Include entry among predictors because the outline permits censoring
  ## conditional on both X and L. Use LT risk sets when fitting the censoring
  ## forest, and preserve rows if predictors need imputation.
  x <- as.data.frame(o$forest$xvar)
  internal <- tail(make.unique(c(names(x), ".rf.time", ".rf.cens", ".rf.entry")), 3)
  x[[internal[3]]] <- entry
  data <- x
  data[[internal[1]]] <- time
  data[[internal[2]]] <- 1 - status
  formula <- as.formula(paste0("Surv(", internal[1], ",", internal[2], ") ~ ."))
  fit <- rfsrc(formula, data, ntree = 50, nsplit = 1,
               splitrule = "random",
               nodesize = set.nodesize(length(time), ncol(x)),
               entry.time = entry, na.action = "na.impute", ntime = 0,
               perf.type = "none")
  prediction <- predict(fit, newdata = x[selected, , drop = FALSE],
                         na.action = "na.impute", perf.type = "none")
  G <- prediction$survival
  knots <- prediction$time.interest
  on.grid <- vapply(seq_along(selected), function(i) {
    evaluate(knots, G[i, ], grid)
  }, numeric(length(grid)))
  ## Keep the grid x subject matrix even for a one-point grid.
  on.grid <- matrix(on.grid, nrow = length(grid), ncol = length(selected))
  list(on.grid = on.grid,
       at.event = vapply(seq_along(selected), function(i) {
         evaluate(knots, G[i, ], time[selected[i]], left = TRUE)
       }, numeric(1)),
       at.entry = vapply(seq_along(selected), function(i) {
         evaluate(knots, G[i, ], entry[selected[i]], left = TRUE)
       }, numeric(1)))
}
