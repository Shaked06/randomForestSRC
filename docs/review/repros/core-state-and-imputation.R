## Rscript docs/review/repros/core-state-and-imputation.R
## R_LIBS may select an isolated current-source installation.
## Reports observed failures without modifying any source or saved results.
suppressPackageStartupMessages(library(randomForestSRC))
options(rf.cores = 1)

## Case 1: growing another forest changes predictions from the original one.
set.seed(18)
n <- 100
d <- data.frame(time = runif(n, 1, 10), status = rbinom(n, 1, .7), x = rnorm(n))
L <- d$time * .8
f <- rfsrc(Surv(time, status) ~ x, d, ntree = 5, entry.time = L,
           save.memory = TRUE, seed = -13)
p1 <- predict(f, newdata = d[, "x", drop = FALSE])
g <- rfsrc(Surv(time, status) ~ x, d, ntree = 5, entry.time = rep(0, n),
           save.memory = TRUE, seed = -13)
p2 <- predict(f, newdata = d[, "x", drop = FALSE])
cat("Same forest, same newdata: max prediction difference after another grow =",
    max(abs(p1$survival - p2$survival)), "(expected 0)\n")

## Case 2: exit-time imputation violates delayed entry and returns negative S.
d <- data.frame(time = c(rep(NA_real_, 90), rep(1, 10)), status = 1, x = 1:100)
L <- c(rep(2, 90), rep(0, 10))
f <- rfsrc(Surv(time, status) ~ x, d, ntree = 1, bootstrap = "none",
           nodesize = 1000, entry.time = L, na.action = "na.impute", seed = -13)
cat("Imputed exits below entry =",
    sum(f$imputed.data[, "time"] < L[f$imputed.indv]), "(expected 0)\n")
cat("Returned survival range =", range(f$survival), "(expected within [0,1])\n")
