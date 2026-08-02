## =====================================================================
##  Reporting — tables and plots
## =====================================================================
##  Presentation only. Nothing here affects a computed result, so this
##  file is deliberately kept OUT of the `cache.extra` hash on the loop
##  chunk: wording, labels, colours, and layout can be changed freely
##  without invalidating a completed run.
##
##  Everything that does affect results lives in sim_engine.R, which IS
##  hashed. Keep that boundary intact — moving a computation in here would
##  let it change silently underneath a cached result.
##
##  Descriptive throughout: values as measured, and the within-arm shift
##  between the two phases. No ranking of one method against another.
## =====================================================================

## The two arms in each family, as named in the write-up. `standard`
## ignores entry times; `left-truncation` uses them in phase 2.
ARM_LABELS <- list(
  cox_std = list(family = "Cox", variant = "standard"),
  cox_lt  = list(family = "Cox", variant = "left-truncation"),
  rsf_std = list(family = "RSF", variant = "standard"),
  rsf_lt  = list(family = "RSF", variant = "left-truncation")
)

arm_label <- function(key, sep = " ") {
  a <- ARM_LABELS[[key]]
  paste(a$family, a$variant, sep = sep)
}


## Mean score per arm in each phase, and the phase-1 -> phase-2 shift.
phase_table <- function(res) {
  m    <- colMeans(res$overall)
  keys <- c("cox_std", "cox_lt", "rsf_std", "rsf_lt")
  data.frame(
    family  = vapply(keys, function(k) ARM_LABELS[[k]]$family,  character(1)),
    variant = vapply(keys, function(k) ARM_LABELS[[k]]$variant, character(1)),
    phase1  = m[paste0("p1_", keys)],
    phase2  = m[paste0("p2_", keys)],
    row.names = NULL
  ) |> transform(shift = phase2 - phase1)
}


## Both phases use the same rows and the same seed, so the phase-1 ->
## phase-2 difference is paired within a replication. Reported as a mean
## with a normal-approximation interval on the replication-level
## differences.
paired_shift <- function(res, conf = 0.95) {
  z <- qnorm(1 - (1 - conf) / 2)
  one <- function(key) {
    dd <- res$overall[[paste0("p2_", key)]] - res$overall[[paste0("p1_", key)]]
    se <- sd(dd) / sqrt(length(dd))
    data.frame(arm = arm_label(key), mean_shift = mean(dd),
               lower = mean(dd) - z * se, upper = mean(dd) + z * se,
               n_moved_down = sum(dd < 0), n_rep = length(dd))
  }
  do.call(rbind, lapply(c("cox_lt", "rsf_lt"), one))
}


## Phase-1 no-op diagnostic: largest gap between a left-truncation arm and
## its standard counterpart, over all replications and all grid points.
##
## The two families warrant different tolerances. entry.time = 0 is
## documented to reproduce the standard forest bit for bit, so the RSF gap
## is held to exact equality. The counting-process Cox with entry 0 has the
## same risk sets as the standard fit but reaches them through a different
## solver path in `survival`, so it is held to numerical agreement.
noop_table <- function(res, cox_tol = 1e-8) {
  data.frame(
    pair    = c(sprintf("Cox standard vs %s (entry 0)", arm_label("cox_lt")),
                sprintf("RSF standard vs %s (entry 0)", arm_label("rsf_lt"))),
    max_gap = c(max(res$noop$cox), max(res$noop$rsf)),
    tol     = c(cox_tol, 0),
    pass    = c(max(res$noop$cox) <= cox_tol, max(res$noop$rsf) == 0)
  )
}


## Score as a function of t, averaged over replications. Shows where along
## the time axis the arms separate, which a single summary figure cannot.
plot_score_by_time <- function(res, main = NULL) {
  keys <- c("cox_std", "cox_lt", "rsf_std", "rsf_lt")
  labs <- vapply(keys, arm_label, character(1), sep = ", ")
  cols <- c("#e6550d", "#31a354", "#e6550d", "#31a354")
  ltys <- c(1, 1, 2, 2)

  Y <- res$by_time[paste0("p2_", keys), , drop = FALSE]
  matplot(res$tgrid, t(Y), type = "l", lwd = 2, col = cols, lty = ltys,
          xlab = "t", ylab = res$scorer_label, las = 1,
          main = if (is.null(main)) {
            sprintf("%s (%d replications)", res$scorer_label, res$n_rep)
          } else main)
  legend("topleft", legend = labs, col = cols, lty = ltys, lwd = 2,
         bty = "n", cex = 0.85)
  invisible(Y)
}


## Both phases of the two left-truncation arms.
plot_phases_box <- function(res) {
  keys <- c("p1_cox_lt", "p2_cox_lt", "p1_rsf_lt", "p2_rsf_lt")
  op <- par(mar = c(6, 4, 3, 1))
  on.exit(par(op), add = TRUE)
  boxplot(res$overall[, keys],
          names = c("Cox\nentry 0", "Cox\nentry used",
                    "RSF\nentry 0", "RSF\nentry used"),
          col   = c("#fdae6b", "#a1d99b", "#fdae6b", "#a1d99b"),
          ylab  = res$scorer_label, las = 1,
          main  = sprintf("Left-truncation arms, both phases (%d replications)",
                          res$n_rep))
  abline(v = 2.5, lty = 3, col = "grey60")
  legend("topright", fill = c("#fdae6b", "#a1d99b"), bty = "n", cex = 0.85,
         legend = c("phase 1: entry zeroed", "phase 2: entry supplied"))
}
