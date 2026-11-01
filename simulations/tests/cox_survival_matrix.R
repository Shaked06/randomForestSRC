## Run from repository root: Rscript simulations/tests/cox_survival_matrix.R
## Compare simulation predictions with survival::survfit, using a literal
## per-time lookup rather than the engine's baseline-hazard index mapping.
source("simulations/_common/sim_engine.R")

reference.survival <- function(fit, newdata, grid) {
  curves <- vapply(seq_len(nrow(newdata)), function(i) {
    sf <- survival::survfit(fit, newdata = newdata[i, , drop = FALSE], se.fit = FALSE)
    vapply(grid, function(tt) {
      observed <- which(sf$time <= tt)
      if (length(observed) == 0) 1 else tail(sf$surv[observed], 1)
    }, numeric(1))
  }, numeric(length(grid)))
  t(matrix(curves, nrow = length(grid), ncol = nrow(newdata)))
}

d <- data.frame(time = c(1, 2, 3, 4, 6, 8, 9, 12),
                status = c(1, 1, 0, 1, 1, 0, 1, 0),
                x = c(-2, 1, 0, -1, 2, 0, 1, -1),
                entry = c(0, .3, .8, 1.1, 2, 3, 4, 5))
tied <- d
tied$time <- c(2, 2, 4, 4, 7, 7, 10, 10)
first.censored <- d
first.censored$status[1] <- 0
fits <- list(
  ordinary = survival::coxph(Surv(time, status) ~ x, d),
  delayed.entry = survival::coxph(Surv(entry, time, status) ~ x, d),
  tied.times = survival::coxph(Surv(time, status) ~ x, tied),
  first.censored = survival::coxph(Surv(time, status) ~ x, first.censored)
)
grids <- list(
  original.failure = c(0, 1, 2, 3, 4, 6, 8, 9, 12),
  multiple.early.times = c(0, .25, .75, 1, 1.5, 2, 2, 3.9, 7, 12, 15),
  entirely.before.followup = c(0, .25, .5),
  entirely.after.followup = c(12, 13, 15),
  no.early.times = c(2, 3, 4, 6, 9, 12),
  single.time = 4,
  unordered.duplicates = c(12, 0, 9, 2, 1, 13, 2, 0)
)
newdata <- data.frame(x = c(-1, 0, 1))
for (fit.name in names(fits)) {
  for (grid.name in names(grids)) {
    fit <- fits[[fit.name]]
    grid <- grids[[grid.name]]
    for (rows in list(1L, seq_len(nrow(newdata)))) {
      nd <- newdata[rows, , drop = FALSE]
      actual <- cox_survival_matrix(fit, as.matrix(nd), grid)
      expected <- reference.survival(fit, nd, grid)
      if (!isTRUE(all.equal(unname(actual), unname(expected), tolerance = 1e-12))) {
        stop("Cox grid mismatch: ", fit.name, "/", grid.name,
             "; maximum difference ", max(abs(actual - expected)))
      }
      stopifnot(identical(dim(actual), c(nrow(nd), length(grid))),
                all(is.finite(actual)), all(actual >= 0 & actual <= 1))
      ordered <- actual[, order(grid), drop = FALSE]
      stopifnot(all(apply(ordered, 1, function(s) all(diff(s) <= 1e-12))))
    }
  }
}
cat("Cox grid regression checks passed: 4 fits, 7 grids, 1 and 3 test subjects.\n")
