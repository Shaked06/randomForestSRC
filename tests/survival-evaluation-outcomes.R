## Regression for bugs 7/8: evaluation outcomes stay separate from grow
## outcomes, including different-sized LT validation samples and subsetting.
library(randomForestSRC)
options(rf.cores = 1)
same <- function(x, y) isTRUE(all.equal(unname(x), unname(y), tolerance = 1e-12))
expect.error <- function(expr, pattern) {
  error <- tryCatch(force(expr), error = identity)
  stopifnot(inherits(error, "error"), grepl(pattern, conditionMessage(error)))
}
set.seed(763)
n <- 60L
train <- data.frame(time = runif(n, 1, 6), status = 1, x = rnorm(n))
train.entry <- runif(n, 0, .8*train$time)
fit <- rfsrc(Surv(time, status) ~ x, train, entry.time = train.entry,
              ntree = 40, ntime = 12, nodesize = 3)
for (n.test in c(15L, n)) {
  test.A <- data.frame(time = seq(101, 102, length.out = n.test),
                       status = 1, x = rnorm(n.test))
  test.B <- test.A
  test.B$time <- seq(.01, .1, length.out = n.test)
  pA <- predict(fit, newdata = test.A)
  pB <- predict(fit, newdata = test.B)
  stopifnot(identical(pA$survival, pB$survival))
  truth.A <- colMeans((1 - pA$survival)^2)
  truth.B <- colMeans(pB$survival^2)
  for (entry in list(NULL, rep(.001, n.test))) {
    a <- get.brier.survival(pA, entry.time = entry)
    b <- get.brier.survival(pB, entry.time = entry)
    stopifnot(same(a$brier.score$brier.score, truth.A),
              same(b$brier.score$brier.score, truth.B),
              !same(a$brier.score$brier.score, b$brier.score$brier.score),
              same(a$eval.event.info$time, test.A$time),
              same(a$grow.event.info$time, train$time))
    subset <- c(n.test, 1L, 3L)
    selected <- get.brier.survival(pA, subset = subset, entry.time = entry)
    stopifnot(same(selected$brier.score$brier.score,
                   colMeans((1 - pA$survival[subset, , drop = FALSE])^2)))
  }
  ## Mixed validation outcomes permit direct case-control comparisons.
  test <- test.A
  test$time[seq(1L, n.test, 2L)] <- .01
  prediction <- predict(fit, newdata = test)
  for (entry in list(NULL, rep(.001, n.test))) {
    auc <- get.auc.survival(prediction, entry.time = entry)
    canonical <- get.auct.survival(prediction, entry.time = entry)
    reference <- vapply(seq_along(prediction$time.interest), function(k) {
      marker <- 1 - prediction$survival[, k]
      case <- test$time <= prediction$time.interest[k]
      credit <- outer(marker[case], marker[!case],
                       function(x, y) (x > y) + .5*(x == y))
      mean(credit)
    }, numeric(1))
    stopifnot(same(auc$auc$auc, reference),
              same(canonical$auct.score$auct, reference))
  }
}
## An unlabeled new-data object must not fall back to grow outcomes.
pNo <- predict(fit, newdata = data.frame(x = rnorm(15)))
for (entry in list(NULL, rep(.1, 15))) {
  expect.error(get.brier.survival(pNo, entry.time = entry), "evaluation outcomes are required")
  expect.error(get.auc.survival(pNo, entry.time = entry), "evaluation outcomes are required")
  expect.error(get.auct.survival(pNo, entry.time = entry), "evaluation outcomes are required")
}
## Ordinary Brier subsetting must be invariant to reordering all subjects.
all <- get.brier.survival(fit)
reverse <- get.brier.survival(fit, subset = n:1)
stopifnot(same(all$brier.score, reverse$brier.score))

## Censoring is trained on grow outcomes/entry times, while loss is evaluated
## on a different-sized validation set. Independently derive product-limit
## censoring survival from counting-process data, on untied continuous times.
set.seed(112)
train <- data.frame(time = runif(n, 2, 10),
                     status = rep(c(0, 1, 1), length.out = n), x = rnorm(n))
train$status[which.max(train$time)] <- 1
train.entry <- runif(n, 0, pmin(1.5, .8*train$time))
fit <- rfsrc(Surv(time, status) ~ x, train, entry.time = train.entry,
              ntree = 40, ntime = 0, nodesize = 3)
test <- data.frame(time = runif(15, 3, 9),
                    status = rep(c(0, 1, 1), 5), x = rnorm(15))
entry <- runif(15, 0, pmin(2, .8*test$time))
prediction <- predict(fit, newdata = test)
grid <- as.numeric(quantile(train$time, c(.3, .4, .6)))
b <- get.brier.survival(prediction, entry.time = entry, times = grid)
sf <- survival::survfit(survival::Surv(train.entry, train$time, 1 - train$status) ~ 1)
G <- function(query, left = FALSE) vapply(query, function(tt) {
  indices <- which(if (left) sf$time < tt else sf$time <= tt)
  if (!length(indices)) 1 else sf$surv[max(indices)]
}, numeric(1))
q <- t(vapply(seq_len(nrow(test)), function(i) {
  index <- which(prediction$time.interest < entry[i])
  divisor <- if (!length(index)) 1 else prediction$survival[i, max(index)]
  vapply(grid, function(tt) {
    index <- which(prediction$time.interest <= tt)
    (if (!length(index)) 1 else prediction$survival[i, max(index)]) / divisor
  }, numeric(1))
}, numeric(length(grid))))
expected <- matrix(NA_real_, nrow(test), length(grid))
for (i in seq_len(nrow(test))) for (k in seq_along(grid)) {
  if (entry[i] > grid[k]) next
  expected[i, k] <- 0
  if (test$status[i] == 1 && test$time[i] <= grid[k]) {
    expected[i, k] <- q[i, k]^2 * G(entry[i], TRUE)/G(test$time[i], TRUE)
  } else if (test$time[i] > grid[k]) {
    expected[i, k] <- (1 - q[i, k])^2 * G(entry[i], TRUE)/G(grid[k])
  }
}
stopifnot(same(b$brier.matx, expected),
          same(b$brier.score$brier.score, colMeans(expected, na.rm = TRUE)),
          same(b$cens.dist, G(grid)), same(b$censoring.at.entry, G(entry, TRUE)))
subset <- c(13, 1, 9, 4)
selected <- get.brier.survival(prediction, entry.time = entry, subset = subset, times = grid)
stopifnot(same(selected$brier.matx, expected[subset, , drop = FALSE]))
## The forest nuisance model also predicts the validation rather than grow rows.
set.seed(14)
b <- get.brier.survival(prediction, entry.time = entry, cens.model = "rfsrc", times = grid)
stopifnot(identical(dim(b$brier.matx), c(15L, 3L)),
          all(is.finite(b$brier.score$brier.score)))
## Reject unsupported LT confidence intervals explicitly.
expect.error(get.brier.survival(prediction, entry.time = entry, conf.int = TRUE),
              "Confidence intervals are not implemented")
