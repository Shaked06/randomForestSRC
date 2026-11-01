## Regression: restored training performance uses the same LT pairs as growing.
library(randomForestSRC)
options(rf.cores = 1)

same <- function(x, y) isTRUE(all.equal(unname(x), unname(y), tolerance = 1e-12))
last.error <- function(fit) unname(tail(fit$err.rate, 1))
concordance <- function(time, status, risk, weight = NULL, entry = NULL) {
  args <- list(time = time, censoring = status, predicted = risk, entry = entry)
  if (!is.null(weight)) args$weight <- weight
  do.call(get.cindex, args)
}
set.seed(6)
n <- 120L
d <- data.frame(time = rexp(n) + 1, status = rbinom(n, 1, .7),
                x = rnorm(n), z = rnorm(n))
entry <- runif(n) * d$time * .95

for (uno in c(FALSE, TRUE)) {
  for (save.memory in c(FALSE, TRUE)) {
    fit <- rfsrc(Surv(time, status) ~ ., d, ntree = 40,
                 entry.time = entry, use.uno = uno,
                 save.memory = save.memory, importance = FALSE, seed = -22)
    weight <- if (uno) fit$forest$uno.weights$weight else NULL
    aware <- concordance(d$time, d$status, fit$predicted.oob,
                        weight = weight, entry = entry)
    blind <- concordance(d$time, d$status, fit$predicted.oob, weight = weight)
    stopifnot(abs(aware - blind) > 1e-4, same(last.error(fit), aware))

    restored <- predict(fit)
    importance <- vimp(fit, importance = "permute", seed = -33)
    forest.only <- predict(fit$forest)
    for (result in list(restored, importance, forest.only)) {
      stopifnot(same(result$predicted.oob, fit$predicted.oob),
                same(last.error(result), aware))
    }

    ## Scoring new subjects must not use the training entry vector, even
    ## if it happens to have the same length. There is no test-entry API yet.
    for (rows in list(seq_len(37), seq_len(n))) {
      test <- d[rows, , drop = FALSE]
      predicted <- predict(fit, newdata = test)
      reference <- concordance(test$time, test$status, predicted$predicted,
                              weight = predicted$uno.weights)
      stopifnot(same(last.error(predicted), reference))
    }

    ## outcome="test" uses native restoration with new outcomes and a zero
    ## entry vector; it must not adopt the original subjects' delayed entry.
    test <- d[seq_len(37), , drop = FALSE]
    pseudo <- predict(fit, newdata = test, outcome = "test")
    reference <- concordance(test$time, test$status, pseudo$predicted.oob,
                            weight = pseudo$uno.weights)
    stopifnot(same(last.error(pseudo), reference))
  }
}

## Ordinary and explicitly zero-entry fits retain the same restored error.
for (uno in c(FALSE, TRUE)) {
  ordinary <- rfsrc(Surv(time, status) ~ ., d, ntree = 40,
                    use.uno = uno, seed = -22)
  zero <- rfsrc(Surv(time, status) ~ ., d, ntree = 40,
                entry.time = rep(0, n), use.uno = uno, seed = -22)
  early <- rfsrc(Surv(time, status) ~ ., d, ntree = 40,
                 entry.time = rep(.1, n), use.uno = uno, seed = -22)
  stopifnot(same(last.error(predict(ordinary)), last.error(ordinary)),
            same(last.error(predict(zero)), last.error(ordinary)),
            same(last.error(predict(early)), last.error(ordinary)))
}
