## Verify algorithm selection deterministically, rather than by timing.
library(randomForestSRC)
options(rf.cores = 1)
same <- function(x, y) isTRUE(all.equal(unname(x), unname(y), tolerance = 1e-12))
score <- function(time, status, risk, entry = NULL, weight = NULL, fast = NULL,
                    algorithm = NULL) {
  args <- list(time = time, censoring = status, predicted = risk,
               entry = entry, do.trace = TRUE)
  if (!is.null(weight)) args$weight <- weight
  if (!is.null(fast)) args$fast <- fast
  trace <- capture.output(value <- do.call(get.cindex, args))
  if (!is.null(algorithm)) {
    stopifnot(any(grepl(paste0("Concordance algorithm: ", algorithm), trace, fixed = TRUE)))
  }
  value
}

set.seed(51)
n <- 600L
time <- sample(1:40, n, replace = TRUE)
status <- sample(c(0, 1), n, replace = TRUE)
risk <- sample(0:10, n, replace = TRUE)
for (weight in list(NULL, sample(c(0, .5, 1, 2), n, replace = TRUE))) {
  ## Automatic selection is fast for large data, with NULL or zero entries.
  ordinary <- score(time, status, risk, weight = weight, algorithm = "Fenwick")
  zero <- score(time, status, risk, rep(0, n), weight, algorithm = "Fenwick")
  slow <- score(time, status, risk, rep(0, n), weight, fast = FALSE,
                 algorithm = "pairwise")
  stopifnot(same(ordinary, zero), same(zero, slow))
  ## Nonzero entries always retain the LT path, including explicit fast=TRUE.
  entry <- .9*time
  active <- score(time, status, risk, entry, weight, algorithm = "pairwise")
  requested.fast <- score(time, status, risk, entry, weight, fast = TRUE,
                           algorithm = "pairwise")
  stopifnot(same(active, requested.fast))
  ## Positive entries before every failure can be inert, but are not
  ## optimized by this conservative all-zero-only change.
  early <- score(time, status, risk, rep(.1, n), weight, algorithm = "pairwise")
  stopifnot(same(ordinary, early))
}

## The existing event-count threshold still selects fast below n=500.
time <- seq_len(400)
status <- rep(1, 400)
risk <- sin(time)
score(time, status, risk, rep(0, 400), algorithm = "Fenwick")
## Small samples honor explicit fast/slow choices and default to pairwise.
for (fast in list(NULL, FALSE, TRUE)) {
  expected <- if (isTRUE(fast)) "Fenwick" else "pairwise"
  score(1:10, rep(1, 10), 10:1, rep(0, 10), fast = fast, algorithm = expected)
}

## Independent pair definition, including tied failures and tied risk scores.
pair.reference <- function(time, status, risk, entry, weight = rep(1, length(time))) {
  total <- loss <- 0
  for (pair in combn(seq_along(time), 2, simplify = FALSE)) {
    i <- pair[1]; j <- pair[2]
    if (anyNA(c(time[pair], status[pair], risk[pair], weight[pair])) ||
        any(weight[pair] == 0)) next
    if (time[i] == time[j] && status[i] == 1 && status[j] == 1) {
      w <- weight[i] + weight[j]
      total <- total + w
      loss <- loss + w * if (risk[i] == risk[j]) 0 else .5
    } else {
      if (time[i] > time[j] || (time[i] == time[j] && status[i] == 0)) {
        swap <- i; i <- j; j <- swap
      }
      if (status[i] != 1 || entry[j] > time[i]) next
      w <- 2*weight[i]
      total <- total + w
      loss <- loss + w*((risk[i] < risk[j]) + .5*(risk[i] == risk[j]))
    }
  }
  if (total == 0) NaN else loss/total
}
time <- c(1, 1, 2, 2, 3, 4, 5, 5)
status <- c(1, 1, 0, 1, 1, 0, 1, 0)
risk <- c(3, 3, 2, 1, 2, 1, NA, 0)
for (weight in list(NULL, c(2, 1, 0, .5, 1, 2, 1, NA))) {
  expected <- pair.reference(time, status, risk, rep(0, 8),
                            if (is.null(weight)) rep(1, 8) else weight)
  for (fast in c(FALSE, TRUE)) {
    stopifnot(same(score(time, status, risk, rep(0, 8), weight, fast = fast), expected))
  }
}
## A real entry restriction changes the answer; fast=TRUE must not ignore it.
for (weight in list(NULL, c(2, 1, 3))) {
  entry <- c(0, 0, 1.5)
  expected <- pair.reference(1:3, rep(1, 3), c(3, 1, 2), entry,
                              if (is.null(weight)) rep(1, 3) else weight)
  actual <- score(1:3, rep(1, 3), c(3, 1, 2), entry, weight,
                    fast = TRUE, algorithm = "pairwise")
  blind <- score(1:3, rep(1, 3), c(3, 1, 2), weight = weight, fast = TRUE)
  stopifnot(same(actual, expected), !same(actual, blind))
}

## Large grow/restore calls use the same dispatcher, not an R-only shortcut.
set.seed(6)
n <- 600L
d <- data.frame(time = runif(n, 1, 10), status = rep(c(0, 1, 1), length.out = n),
                  x = rnorm(n))
for (uno in c(FALSE, TRUE)) {
  grow <- function(entry = NULL) {
    capture.output(fit <- rfsrc(Surv(time, status) ~ x, d, entry.time = entry,
                                 ntree = 12, nodesize = 5, use.uno = uno,
                                 do.trace = TRUE, seed = -22))
    fit
  }
  trace <- capture.output(ordinary <- rfsrc(Surv(time, status) ~ x, d,
                              ntree = 12, nodesize = 5, use.uno = uno,
                              do.trace = TRUE, seed = -22))
  stopifnot(any(grepl("Concordance algorithm: Fenwick", trace, fixed = TRUE)))
  zero <- grow(rep(0, n))
  stopifnot(same(ordinary$survival, zero$survival), same(ordinary$err.rate, zero$err.rate))
  trace <- capture.output(restored <- predict(ordinary, do.trace = TRUE))
  stopifnot(any(grepl("Concordance algorithm: Fenwick", trace, fixed = TRUE)),
            same(tail(ordinary$err.rate, 1), tail(restored$err.rate, 1)))
  trace <- capture.output(active <- rfsrc(Surv(time, status) ~ x, d,
                             entry.time = .8*d$time, ntree = 12, nodesize = 5,
                             use.uno = uno, do.trace = TRUE, seed = -22))
  stopifnot(any(grepl("Concordance algorithm: pairwise", trace, fixed = TRUE)))
  expected <- score(d$time, d$status, active$predicted.oob, .8*d$time,
                      if (uno) active$forest$uno.weights$weight else NULL, fast = FALSE)
  stopifnot(same(tail(active$err.rate, 1), expected))
}
