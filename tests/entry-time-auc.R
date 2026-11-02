## Entry-restricted C/D AUC: no source-entry-CDF weights are required.
library(randomForestSRC)
options(rf.cores = 1)

auc.object <- function(time, status, S, grid, x = seq_along(time)) {
  mortality <- rowSums(1 - S)
  o <- list(family = "surv", yvar = cbind(time, status),
            predicted = mortality, predicted.oob = mortality,
            survival.oob = S, time.interest = grid,
            forest = list(yvar = cbind(time, status), xvar = data.frame(x = x)),
            imputed.indv = NULL)
  class(o) <- c("rfsrc", "grow", "surv")
  o
}
same <- function(x, y) isTRUE(all.equal(unname(x), unname(y), tolerance = 1e-12))

## Exact retained sample from independent source T uniform on {1,2,3}
## and L uniform on {0,1.5}. At t=2 there are three cases and two controls.
## Conditional markers are 2/3 for early and 1/2 for late entrants.
## Pairwise credit is 3.5/6 = 7/12. Legacy entry weights give 27/44.
time <- c(1, 2, 3, 2, 3)
entry <- c(0, 0, 0, 1.5, 1.5)
S <- matrix(rep(c(2/3, 1/3, 0), each = 5), 5)
o <- auc.object(time, rep(1, 5), S, 1:3)
a <- get.auc.survival(o, entry.time = entry)
stopifnot(same(a$auc$auc[2], 7/12), is.null(a$GL))

## Independent quadratic reference for the sorted weighted-pair algorithm.
## At each time, use explicit entered cases/controls and half credit for ties.
pair.reference <- function(marker, case, control, w.case, w.control) {
  if (!any(case) || !any(control)) return(NA_real_)
  credit <- outer(marker[case], marker[control], function(x, y) (x > y) + .5*(x == y))
  weights <- outer(w.case[case], w.control[control])
  sum(credit * weights) / sum(weights)
}
reference <- function(time, status, entry, S, grid, G.entry, G.event, G.grid) {
  conditional <- vapply(seq_along(time), function(i) {
    before <- which(grid < entry[i])
    divisor <- if (length(before)) S[i, max(before)] else 1
    S[i, ] / divisor
  }, numeric(length(grid)))
  vapply(seq_along(grid), function(k) {
    marker <- 1 - conditional[k, ]
    eligible <- entry <= grid[k] & is.finite(marker)
    case <- eligible & time <= grid[k] & status == 1
    control <- eligible & time > grid[k]
    pair.reference(marker, case, control, G.entry/G.event, G.entry/G.grid[k, ])
  }, numeric(1))
}
expected <- reference(time, rep(1, 5), entry, S, 1:3,
                      rep(1, 5), rep(1, 5), matrix(1, 3, 5))
stopifnot(same(a$auc$auc, expected))
permuted <- get.auc.survival(o, subset = 5:1, entry.time = entry)
stopifnot(same(a$auc, permuted$auc))

## Censoring at 2 has delayed-entry risk size 3. Early subjects need
## weight exp(1/3) at t=4, late entrants need weight one. The two case
## markers are .8,.4 and the two control markers .6,.2.
time <- c(2, 4, 6, 4, 6)
status <- c(0, 1, 1, 1, 1)
entry <- c(0, 0, 0, 3, 3)
grid <- c(1, 2, 3, 4, 6)
S <- matrix(rep(c(.5, .2, .4, .6, .8), length(grid)), 5)
S[4:5, 1:2] <- 1
o <- auc.object(time, status, S, grid)
a <- get.auc.survival(o, entry.time = entry)
g <- exp(-1/3)
G.grid <- matrix(rep(c(1, rep(g, 4)), 5), 5)
expected <- reference(time, status, entry, S, grid,
                      c(1, 1, 1, g, g), c(1, g, g, g, g), G.grid)
w <- 1/g
stopifnot(same(a$auc$auc, expected),
          same(a$auc$auc[4], (w*w + w + 1)/(w + 1)^2),
          same(a$cens.dist, c(1, rep(g, 4))))

## Event weights use G(U^-), including when a failure ties censoring.
## One early case outranks the control; the later case does not.
time <- c(2, 2, 3, 4)
status <- c(0, 1, 1, 1)
entry <- rep(0, 4)
S <- matrix(rep(c(.5, .2, .8, .5), 4), 4)
o <- auc.object(time, status, S, 1:4)
a <- get.auc.survival(o, entry.time = entry)
stopifnot(same(a$auc$auc[3], 1/(1 + exp(.25))))

## Inclusive entry/event ties retain the earliest case, rather than losing
## it to an infinite entry-CDF weight. All tied markers yield AUC = 1/2.
o <- auc.object(1:4, rep(1, 4), matrix(.5, 4, 4), 1:4)
a <- get.auc.survival(o, entry.time = rep(1, 4))
stopifnot(same(a$auc$auc[1:3], rep(.5, 3)), is.na(a$auc$auc[4]))

## No entrants, cases, or controls gives NA. A single case/control pair
## on a one-point grid retains dimensions and receives half credit for ties.
o <- auc.object(c(2, 3), c(1, 1), matrix(.5, 2, 1), 2)
a <- get.auc.survival(o, entry.time = c(2, 2))
stopifnot(same(a$auc$auc, .5), is.na(a$iauc))
o <- auc.object(c(2, 3), c(1, 1), matrix(.5, 2, 3), c(0, 1, 2))
a <- get.auc.survival(o, entry.time = c(.5, 1.5))
stopifnot(all(is.na(a$auc$auc[1:2])), same(a$auc$auc[3], .5))

## A subject who has not entered cannot serve as a control.
o <- auc.object(c(1, 3, 4), rep(1, 3), rbind(c(.5, .5), c(.5, .5), c(.9, .9)), c(1, 2))
a <- get.auc.survival(o, entry.time = c(0, 0, 3))
stopifnot(same(a$auc$auc, c(.5, .5)))

## Ordinary NULL entry and explicit zero entry agree without censoring.
## Missing OOB predictions are excluded from pair comparisons.
o <- auc.object(1:4, rep(1, 4), rbind(c(.4,.3,.2), c(.6,.5,.3),
                                    c(.8,.6,.4), c(NA,NA,NA)), 1:3)
a <- get.auc.survival(o, entry.time = rep(0, 4))
stopifnot(same(a$auc, get.auc.survival(o)$auc))

## Independent cumulative-hazard estimates and explicit pair matrices on
## small random fixtures check weighted sorting, ties, and off-grid times.
set.seed(72)
for (iteration in 1:8) {
  n <- 12L
  time <- runif(n, .6, 6)
  status <- rep(c(0, 1, 1), length.out = n)
  entry <- runif(n, 0, time - .1)
  grid <- seq(.5, 6, .5)
  rate <- sample(c(.1, .2, .3), n, replace = TRUE)
  S <- exp(-outer(rate, grid))
  o <- auc.object(time, status, S, grid)
  a <- get.auc.survival(o, entry.time = entry)
  sf <- survival::survfit(survival::Surv(entry, time, 1 - status) ~ 1)
  G <- function(query, left = FALSE) vapply(query, function(tt) {
    indices <- which(if (left) sf$time < tt else sf$time <= tt)
    if (!length(indices)) 1 else exp(-sf$cumhaz[max(indices)])
  }, numeric(1))
  expected <- reference(time, status, entry, S, grid,
                        G(entry, TRUE), G(time, TRUE),
                        matrix(rep(G(grid), n), length(grid)))
  stopifnot(same(a$auc$auc, expected))
}

## Exercise the public scorer on an actual delayed-entry grow object,
## checking its OOB curves against the independent pairwise reference.
set.seed(29)
n <- 80L
time <- runif(n, 1, 10)
status <- rep(c(0, 1, 1, 1), length.out = n)
entry <- runif(n, 0, pmin(2, .8*time))
data <- data.frame(time = time, status = status, x = rnorm(n))
fit <- rfsrc(Surv(time, status) ~ x, data, entry.time = entry,
              ntree = 30, ntime = 8, perf.type = "none")
a <- get.auc.survival(fit, entry.time = entry)
sf <- survival::survfit(survival::Surv(entry, time, 1 - status) ~ 1)
G <- function(query, left = FALSE) vapply(query, function(tt) {
  indices <- which(if (left) sf$time < tt else sf$time <= tt)
  if (!length(indices)) 1 else exp(-sf$cumhaz[max(indices)])
}, numeric(1))
grid <- fit$time.interest
expected <- reference(time, status, entry, fit$survival.oob, grid,
                      G(entry, TRUE), G(time, TRUE),
                      matrix(rep(G(grid), n), length(grid)))
stopifnot(any(is.finite(a$auc$auc)), same(a$auc$auc, expected))

## Forest censoring shares entry-conditioned probabilities with Brier,
## using the same RNG seed. Missing predictors preserve observation rows.
set.seed(31)
n <- 120L
time <- runif(n, 2, 10)
status <- rep(c(1, 1, 0), length.out = n)
entry <- runif(n, .1, .8*time)
x <- seq_len(n)
x[c(3, 17)] <- NA
grid <- c(1, 2, 3, 4, 5)
o <- auc.object(time, status, matrix(.7, n, length(grid)), grid, x)
set.seed(13)
a <- get.auc.survival(o, entry.time = entry, cens.model = "rfsrc")
set.seed(13)
b <- get.brier.survival(o, entry.time = entry, cens.model = "rfsrc")
stopifnot(identical(dim(a$surv.ensb), c(length(grid), n)),
          same(a$cens.dist, b$cens.dist),
          same(a$censoring.at.entry, b$censoring.at.entry),
          all(a$auc$auc >= 0 & a$auc$auc <= 1, na.rm = TRUE))
