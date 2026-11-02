## Professor's outline, section 9.2: predictions conditional on observation,
## and IPCW based on delayed-entry censoring risk sets.
library(randomForestSRC)
options(rf.cores = 1)

make.object <- function(time, status, survival, grid, x = seq_along(time)) {
  mortality <- rowSums(1 - survival)
  o <- list(family = "surv", yvar = cbind(time, status),
            predicted = mortality, predicted.oob = mortality,
            survival.oob = survival, time.interest = grid,
            forest = list(yvar = cbind(time, status), xvar = data.frame(x = x)),
            imputed.indv = NULL)
  class(o) <- c("rfsrc", "grow", "surv")
  o
}
same <- function(x, y) isTRUE(all.equal(unname(x), unname(y), tolerance = 1e-12))

## With no censoring, this must be plain squared error among entered subjects:
## using entry-conditioned predictions and no truncation-CDF weights.
grid <- c(1, 2, 3)
S <- rbind(c(.8, .6, .4), c(.9, .7, .5), c(.95, .85, .75))
o <- make.object(c(2, 3, 4), rep(1, 3), S, grid)
entry <- c(.5, 1.5, 2.5)
b <- get.brier.survival(o, entry.time = entry)
expected <- matrix(c(.04, NA, NA, .36, (1 - .7/.9)^2, NA,
                     .16, (.5/.9)^2, (1 - .75/.85)^2), 3)
stopifnot(same(b$brier.matx, expected),
          same(b$brier.score$brier.score, colMeans(expected, na.rm = TRUE)),
          same(b$surv.ensb, t(S)), is.null(b$GL), same(b$n.eligible, 1:3))
no.censor.forest <- get.brier.survival(o, entry.time = entry, cens.model = "rfsrc")
stopifnot(same(no.censor.forest$brier.score, b$brier.score))

## Hand-computed delayed-entry censoring example. At the censoring time 2,
## the people entering at 3 are excluded: Y_C(2)=2 and G(2)=exp(-1/2).
## Their conditional censoring weight is 1 thereafter; the subject entered
## at 1 has weight exp(1/2). Predictions are 1 before entry and .5 afterward.
grid <- c(2, 3, 4)
S <- matrix(.5, 4, 3)
S[3:4, 1] <- 1
o <- make.object(c(2, 4, 5, 6), c(0, 1, 1, 1), S, grid)
entry <- c(0, 1, 3, 3)
b <- get.brier.survival(o, entry.time = entry)
g <- exp(-.5)
expected <- rbind(c(0, 0, 0), rep(.25/g, 3), c(NA, .25, .25), c(NA, .25, .25))
stopifnot(same(b$brier.matx, expected),
          same(b$brier.score$brier.score, colMeans(expected, na.rm = TRUE)),
          same(b$cens.dist, rep(g, 3)))

## Look up weights at the actual censoring times, even if they are absent
## from the score grid. An event uses G(U^-), a surviving subject G(t).
grid <- c(2, 5)
S <- matrix(.5, 4, 2)
S[3:4, 1] <- 1
o <- make.object(c(2, 4, 5, 6), c(0, 1, 1, 1), S, grid)
b <- get.brier.survival(o, entry.time = entry)
stopifnot(same(b$brier.score$brier.score, colMeans(expected[, c(1, 3)], na.rm = TRUE)))

## Zero entry reduces to the ordinary scorer's censoring estimator and score.
## Censoring at time 2 has risk size 4, giving G=exp(-1/4) afterward.
o <- make.object(c(2, 4, 5, 6), c(0, 1, 1, 1), matrix(.5, 4, 2), grid)
b <- get.brier.survival(o, entry.time = rep(0, 4))
stopifnot(same(b$cens.dist, rep(exp(-.25), 2)),
          same(b$brier.score$brier.score, rep(3/(16*exp(-.25)), 2)),
          same(b$brier.score, get.brier.survival(o)$brier.score))

## Entry/event ties are permitted by the outline's inclusive risk indicator.
## With no censoring there must be no infinite entry-CDF weights.
o <- make.object(1:4, rep(1, 4), matrix(.5, 4, 4), 1:4)
b <- get.brier.survival(o, entry.time = rep(1, 4))
stopifnot(all(is.finite(b$brier.matx)), same(b$brier.score$brier.score, rep(.25, 4)))

## No subject entered: explicitly undefined; integration must not bridge it.
o <- make.object(c(2, 3, 4), rep(1, 3), matrix(.5, 3, 3), c(0, 1, 2))
b <- get.brier.survival(o, entry.time = c(.5, 1.5, 2.5))
stopifnot(is.na(b$brier.score$brier.score[1]), is.na(b$crps))

## LT subject selection must align outcomes, predictions, and entries.
all <- get.brier.survival(o, entry.time = c(.5, 1.5, 2.5))
permuted <- get.brier.survival(o, subset = 3:1, entry.time = c(.5, 1.5, 2.5))
stopifnot(same(all$brier.score$brier.score, permuted$brier.score$brier.score))

## A single subject and one score point preserve matrix dimensions.
o <- make.object(c(2, 3), c(1, 1), matrix(.5, 2, 1), 2)
b <- get.brier.survival(o, subset = 1, entry.time = c(2, 2))
stopifnot(identical(dim(b$brier.matx), c(1L, 1L)),
          identical(dim(b$conditional.survival), c(1L, 1L)),
          same(b$brier.score$brier.score, .25))

## Missing OOB predictions do not count toward the scoring denominator.
o <- make.object(c(2, 3), c(1, 1), rbind(c(.5, .5), c(NA, NA)), c(1, 2))
b <- get.brier.survival(o, entry.time = c(0, 0))
stopifnot(same(b$n.eligible, c(1, 1)), same(b$brier.score$brier.score, c(.25, .25)))

## Conditioning on a model-impossible entry must fail explicitly.
o <- make.object(c(2, 3), c(1, 1), matrix(0, 2, 2), c(1, 2))
err <- tryCatch(get.brier.survival(o, entry.time = c(1.5, 1.5)), error = identity)
stopifnot(inherits(err, "error"), grepl("Survival at entry is zero", conditionMessage(err)))

## Censoring cumulative-hazard reference, independently from survfit.
## These times are untied and the entry times do not tie censoring times.
time <- c(2, 4, 5, 6, 7, 9)
status <- c(0, 1, 0, 1, 1, 1)
entry <- c(.1, .5, 3, 3, 4.5, 5.5)
grid <- c(1, 2, 3, 4, 5, 7, 8)
o <- make.object(time, status, matrix(.5, length(time), length(grid)), grid)
b <- get.brier.survival(o, entry.time = entry)
sf <- survival::survfit(survival::Surv(entry, time, 1 - status) ~ 1)
expected <- vapply(grid, function(tt) {
  k <- which(sf$time <= tt)
  if (length(k) == 0) 1 else exp(-sf$cumhaz[max(k)])
}, numeric(1))
stopifnot(same(b$cens.dist, expected))

## Exact oracle check for the entered cohort: the source distribution has
## masses .25, .25, .5 at times .1, .3, .8. Given entry at .25, survival to
## .5 is 2/3. A representative cohort has 100 failures and 200 survivors.
time <- c(rep(.3, 100), rep(.8, 200))
entry <- rep(.25, 300)
grid <- c(.1, .3, .5, .8)
oracle <- matrix(rep(c(.75, .5, .5, 0), each = 300), 300)
o <- make.object(time, rep(1, 300), oracle, grid)
b <- get.brier.survival(o, entry.time = entry)
wrong <- make.object(time, rep(1, 300), oracle^1.3, grid)
bad <- get.brier.survival(wrong, entry.time = entry)
stopifnot(same(b$brier.score$brier.score[3], 2/9),
          bad$brier.score$brier.score[3] > b$brier.score$brier.score[3])

## Exact representative truncated sample from independent source variables:
## T is uniform on {1,2,3}; L is uniform on {0,1.5}; retain L <= T.
## The five retained combinations are equally likely. At t=2 the true
## conditional probabilities are 1/3 for early and 1/2 for late entrants.
## The old GL weights preferred the distorted S^1.1 over the oracle here.
time <- c(1, 2, 3, 2, 3)
entry <- c(0, 0, 0, 1.5, 1.5)
oracle <- matrix(rep(c(2/3, 1/3, 0), each = 5), 5)
o <- make.object(time, rep(1, 5), oracle, 1:3)
wrong <- make.object(time, rep(1, 5), oracle^1.1, 1:3)
b <- get.brier.survival(o, entry.time = entry)
bad <- get.brier.survival(wrong, entry.time = entry)
stopifnot(same(b$brier.score$brier.score[2], 7/30),
          bad$brier.score$brier.score[2] > b$brier.score$brier.score[2])

## The forest censoring model also receives entry times and preserves rows.
set.seed(31)
n <- 120L
time <- runif(n, 2, 10)
status <- rep(c(1, 1, 0), length.out = n)
entry <- runif(n, .1, .8 * time)
grid <- c(1, 2, 3)
x <- seq_len(n)
x[c(3, 17)] <- NA
o <- make.object(time, status, matrix(.7, n, length(grid)), grid, x = x)
set.seed(13)
b <- get.brier.survival(o, entry.time = entry, cens.model = "rfsrc")
stopifnot(identical(dim(b$brier.matx), c(n, length(grid))),
          all(is.finite(b$brier.score$brier.score)), all(b$cens.dist >= 0))
