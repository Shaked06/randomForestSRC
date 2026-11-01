## Independent VIMP reference: in a tree split once on binary x, anti-VIMP
## sends each subject to the leaf reached by replacing x with 1-x.
library(randomForestSRC)
options(rf.cores = 1)

same <- function(x, y) isTRUE(all.equal(unname(x), unname(y), tolerance = 1e-12))
set.seed(6)
n <- 120L
x <- rep(0:1, each = n / 2)
d <- data.frame(time = rexp(n, rate = exp(x)) + 1,
                status = rbinom(n, 1, .7), x = x)
entry <- runif(n) * d$time * .95

for (uno in c(FALSE, TRUE)) {
  fit <- rfsrc(Surv(time, status) ~ x, d, ntree = 1, nodesize = 3, nsplit = 0,
               membership = TRUE, entry.time = entry, use.uno = uno, seed = -22)
  stopifnot(fit$leaf.count == 2)
  oob <- fit$inbag[, 1] == 0
  swapped <- predict(fit, newdata = data.frame(x = 1 - x))
  weight <- if (uno) fit$forest$uno.weights$weight[oob] else NULL
  error <- function(prediction, with.entry = TRUE) {
    args <- list(time = d$time[oob], censoring = d$status[oob],
                 predicted = prediction[oob],
                 entry = if (with.entry) entry[oob] else NULL)
    if (!is.null(weight)) args$weight <- weight
    do.call(get.cindex, args)
  }
  baseline <- error(fit$predicted.oob)
  expected <- error(swapped$predicted) - baseline
  blind <- error(swapped$predicted, FALSE) - error(fit$predicted.oob, FALSE)
  stopifnot(abs(expected - blind) > 1e-4)

  v <- vimp(fit, importance = "anti", block.size = 1, seed = -33)
  joint <- vimp(fit, importance = "anti", block.size = 1, joint = TRUE, seed = -33)
  stopifnot(same(v$importance, expected), same(joint$importance, expected),
            same(tail(v$err.rate, 1), baseline),
            same(tail(joint$err.rate, 1), baseline))

  ## Both interval procedures calculate missing reference importance using
  ## standalone VIMP. Check their individual, joint, and error references.
  ## Use the default Uno setting so refits and the source use the same setting.
  if (uno) {
    for (bootstrap in c(FALSE, TRUE)) {
      set.seed(23)
      s <- subsample(fit, B = 2, block.size = 1, importance = "anti",
                     subratio = .7, stratify = FALSE, performance = TRUE,
                     joint = TRUE, bootstrap = bootstrap, verbose = FALSE)
      reference <- s$vmp[[1]][, 1]
      stopifnot(same(reference["x"], expected),
                same(reference["joint"], expected),
                same(reference["err"], baseline),
                same(s$rf$importance, expected))
    }
  }
}
