## Positive entry times must not silently select an entry-blind split rule.
library(randomForestSRC)
options(rf.cores = 1)
same <- function(x, y) isTRUE(all.equal(x, y, tolerance = 1e-12))

set.seed(6)
n <- 120L
d <- data.frame(time = rexp(n) + 1, status = rbinom(n, 1, .7),
                  x = rnorm(n), z = rnorm(n))
entry <- runif(n)*d$time*.95
fit <- function(rule = NULL, entries = NULL, data = d, ...) {
  rfsrc(Surv(time, status) ~ x + z, data, ntree = 5, nodesize = 5,
        splitrule = rule, entry.time = entries, seed = -13, ...)
}
expect.unsupported <- function(expr) {
  result <- tryCatch(force(expr), error = identity)
  if (!inherits(result, "error")) {
    stop("An unsupported split rule silently accepted positive entry times")
  }
  message <- conditionMessage(result)
  stopifnot(grepl("Positive entry.time", message, fixed = TRUE),
            grepl("splitrule", message, fixed = TRUE),
            grepl('"logrank"', message, fixed = TRUE),
            grepl('"random"', message, fixed = TRUE))
  invisible(result)
}

for (rule in c("logrankscore", "bs.gradient", "custom", "custom1")) {
  expect.unsupported(fit(rule, entry))
  ## One positive value is sufficient, even before the first failure.
  one <- rep(0, n); one[1] <- .1
  expect.unsupported(fit(rule, one))
  expect.unsupported(fit(rule, rep(.1, n)))
  ## Disabling performance/output/splitting does not promise rule support.
  expect.unsupported(fit(rule, entry, perf.type = "none", forest = FALSE,
                         nodedepth = 0))
  ## Ordinary right-censored behavior remains available for these rules.
  for (uno in c(FALSE, TRUE)) {
    ordinary <- fit(rule, use.uno = uno)
    zero <- fit(rule, rep(0, n), use.uno = uno)
    stopifnot(same(ordinary$survival, zero$survival),
              same(ordinary$chf, zero$chf),
              same(ordinary$predicted.oob, zero$predicted.oob),
              same(ordinary$err.rate, zero$err.rate),
              !is.null(ordinary$forest$nativeArray),
              identical(ordinary$forest$nativeArray, zero$forest$nativeArray),
              identical(ordinary$forest$nativeFactorArray, zero$forest$nativeFactorArray),
              identical(ordinary$forest$leafCount, zero$forest$leafCount),
              is.null(ordinary$entry.time), identical(zero$entry.time, rep(0, n)))
  }
}

## Check the resolved rule, including the package's existing abbreviations.
expect.unsupported(fit("logranks", entry))
expect.unsupported(fit("bs.g", entry))
default <- fit(entries = entry)
explicit <- fit("logrank", entry)
stopifnot(default$splitrule == "logrank",
          same(default$survival, explicit$survival),
          same(default$err.rate, explicit$err.rate))
for (rule in c("logrank", "random", "rand")) {
  for (memory in c(FALSE, TRUE)) {
    active <- fit(rule, entry, save.memory = memory)
    stopifnot(identical(active$entry.time, entry),
              identical(active$forest$entry.time, entry),
              all(is.finite(active$survival)),
              all(active$survival >= 0 & active$survival <= 1))
    restored <- predict(active)
    stopifnot(same(tail(active$err.rate, 1), tail(restored$err.rate, 1)))
  }
}

## Validation follows the retained rows: an omitted subject's positive entry
## must not restrict the ordinary forest that remains.
missing <- d; missing$x[1] <- NA_real_
one <- rep(0, n); one[1] <- .1
for (rule in c("logrankscore", "bs.gradient", "custom1")) {
  omitted <- fit(rule, one, data = missing)
  cleaned <- fit(rule, rep(0, n - 1L), data = d[-1, ])
  stopifnot(omitted$n == n - 1L, same(omitted$survival, cleaned$survival),
            identical(omitted$entry.time, rep(0, n - 1L)))
  retained <- one; retained[2] <- .1
  expect.unsupported(fit(rule, retained, data = missing))
  expect.unsupported(fit(rule, one, data = missing, na.action = "na.impute"))
}

## The fast wrapper must delegate to the same grow-time validation.
expect.unsupported(rfsrc.fast(Surv(time, status) ~ x + z, d, ntree = 5,
                              splitrule = "logrankscore", entry.time = entry))
fast <- rfsrc.fast(Surv(time, status) ~ x + z, d, ntree = 5,
                   splitrule = "random", entry.time = entry)
stopifnot(identical(fast$entry.time, entry))

## Multiple event types resolve logrank to a different native rule. That
## rule is outside the supported LT splitting set, as is logrankCR itself.
cr <- d; cr$status <- rep(c(0, 1, 2), length.out = n)
for (rule in list(NULL, "logrank", "logrankCR")) {
  expect.unsupported(fit(rule, entry, data = cr))
  ordinary <- fit(rule, data = cr)
  zero <- fit(rule, rep(0, n), data = cr)
  stopifnot(same(ordinary$chf, zero$chf),
            same(ordinary$err.rate, zero$err.rate))
}
