## Practical entry-restricted IPCW score from the project outline, section 9.2.
## See docs/brier_score_left_truncation.md for the conditioning and assumptions.
get.brier.survival.lt <- function(o, subset, cens.model, papply, entry.time) {
  setup <- survival.score.setup.lt(o, subset, cens.model, papply, entry.time)
  info <- setup$event.info
  time <- info$time
  status <- info$cens
  grid <- info$time.interest
  selected <- setup$subset
  raw.S <- setup$surv.ensb
  S <- setup$conditional.survival
  entry <- setup$entry.selected
  censoring <- setup$censoring
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
