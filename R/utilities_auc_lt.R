## Entered-cohort extension of the observation-conditioning interpretation
## used for Brier. The professor's outline does not specify an AUC estimator.
get.auc.survival.lt <- function(o, subset, cens.model, papply, entry.time, times = NULL) {
  setup <- survival.score.setup.lt(o, subset, cens.model, papply, entry.time, times)
  info <- setup$event.info
  grid <- info$time.interest
  tau <- setup$time.selected
  event <- setup$status.selected
  S <- setup$conditional.survival
  censoring <- setup$censoring
  auc <- unlist(papply(seq_along(grid), function(k) {
    ## Before-entry predictions are already NA. Missing OOB predictions
    ## are excluded from pair comparisons at the relevant time.
    marker <- 1 - S[k, ]
    eligible <- is.finite(marker)
    case <- eligible & tau <= grid[k] & event == 1
    control <- eligible & tau > grid[k]
    if (!any(case) || !any(control)) return(NA_real_)
    gc.case <- censoring$at.event[case]
    gc.control <- if (cens.model == "km") {
      rep(censoring$on.grid[k], sum(control))
    } else {
      censoring$on.grid[k, control]
    }
    ge.case <- censoring$at.entry[case]
    ge.control <- censoring$at.entry[control]
    required <- c(gc.case, gc.control, ge.case, ge.control)
    if (any(!is.finite(required) | required <= 0)) {
      stop("Censoring survival is zero or undefined where the AUC needs it; restrict the evaluation horizon")
    }
    ## IPCW conditional on reaching entry. A common factor cancels within
    ## each group, so normalize log weights to avoid overflow in pair sums.
    case.logw <- log(ge.case) - log(gc.case)
    control.logw <- log(ge.control) - log(gc.control)
    wi <- exp(case.logw - max(case.logw))
    wj <- exp(control.logw - max(control.logw))
    mi <- marker[case]
    mj <- marker[control]
    ## Sorted weighted Mann--Whitney calculation: avoid a quadratic pair
    ## matrix, retaining half credit when case and control markers tie.
    ord <- order(mj)
    mj <- mj[ord]
    cw <- c(0, cumsum(wj[ord]))
    W.le <- cw[1L + findInterval(mi, mj)]
    W.lt <- cw[1L + findInterval(mi, mj, left.open = TRUE)]
    sum(wi * (W.lt + .5 * (W.le - W.lt))) / (sum(wi) * sum(wj))
  }))
  ## Retain the historical descriptive time-average over defined points.
  ok <- is.finite(auc)
  iauc <- if (sum(ok) >= 2 && diff(range(grid[ok])) > 0) {
    trapz(grid[ok], auc[ok]) / diff(range(grid[ok]))
  } else {
    NA_real_
  }
  list(auc = data.frame(time = grid, auc = auc), iauc = iauc,
       cens.dist = censoring$on.grid, time = grid, event.info = info,
       grow.event.info = setup$grow.event.info,
       eval.event.info = setup$eval.event.info,
       subset = setup$subset, mort = setup$mort,
       surv.ensb = S, GL = NULL,
       score.type = "entry-restricted IPCW cumulative/dynamic AUC",
       censoring.at.entry = censoring$at.entry)
}
