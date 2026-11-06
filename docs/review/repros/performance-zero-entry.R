## A timing illustration; elapsed-time ratios depend on the machine.
## The exact equality check is deterministic; timing is not a regression test.
suppressPackageStartupMessages(library(randomForestSRC))
set.seed(42)
n <- 10000
time <- rexp(n)
status <- rbinom(n, 1, .7)
risk <- rnorm(n)
a <- system.time(x <- get.cindex(time, status, risk))[["elapsed"]]
b <- system.time(y <- get.cindex(time, status, risk, entry = rep(0, n)))[["elapsed"]]
stopifnot(isTRUE(all.equal(x, y)))
cat("Concordance with NULL entry:", a, "s; with all-zero entry:", b,
    "s; values equal:", x, "\n")
cat("Grow calls pass a non-NULL entry vector even when entry.time is omitted;\n",
    "src/survivalE.c disables the Fenwick path whenever entry != NULL.\n", sep = "")
