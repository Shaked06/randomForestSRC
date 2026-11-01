## Regression: prediction must use this forest's entry times, not the last fit.
library(randomForestSRC)
options(rf.cores = 1)

check.curves <- function(actual, expected) {
  stopifnot(isTRUE(all.equal(actual$survival, expected$survival, tolerance = 1e-12)),
            isTRUE(all.equal(actual$chf, expected$chf, tolerance = 1e-12)))
}

set.seed(18)
n <- 100L
d <- data.frame(time = runif(n, 1, 10), status = rbinom(n, 1, .7),
                x = rnorm(n))
entry <- .8 * d$time
for (save.memory in c(FALSE, TRUE)) {
  fit <- rfsrc(Surv(time, status) ~ x, d, ntree = 5, entry.time = entry,
               save.memory = save.memory, seed = -13)
  expected <- list(survival = fit$survival, chf = fit$chf)
  check.curves(predict(fit, newdata = d["x"]), expected)

  ## The second fit deliberately uses different entry times and owns a
  ## different vector. Garbage collection must not affect prediction either.
  other <- rfsrc(Surv(time, status) ~ x, d, ntree = 5, seed = -13)
  invisible(gc())
  check.curves(predict(fit, newdata = d["x"]), expected)
  check.curves(predict(fit$forest, newdata = d["x"]), expected)
  check.curves(predict(fit), expected)

  ## Previously saved grow objects have entry.time only on the outer object.
  legacy <- fit
  legacy$forest$entry.time <- NULL
  check.curves(predict(legacy, newdata = d["x"]), expected)

  ## Ordinary forests without a stored entry field retain zero-entry behavior.
  zero <- other
  zero$entry.time <- NULL
  zero$forest$entry.time <- NULL
  check.curves(predict(zero$forest, newdata = d["x"]),
               list(survival = zero$survival, chf = zero$chf))
}
