## Reproduce the saved-survival-forest prediction failure in two R processes.
## Rscript docs/review/repros/core-grow.R /tmp/review-survival-fit.rds
## Rscript docs/review/repros/core-predict.R /tmp/review-survival-fit.rds > /tmp/review-predict.log 2>&1
## R_LIBS may select an isolated current-source installation.
suppressPackageStartupMessages(library(randomForestSRC))
options(rf.cores = 1)
args <- commandArgs(trailingOnly = TRUE)
stopifnot(length(args) == 1L)
set.seed(1)
d <- data.frame(time = seq_len(50), status = rep(0:1, 25), x = rnorm(50))
## Deliberately omit entry.time: ordinary survival forests are affected too.
f <- rfsrc(Surv(time, status) ~ x, d, ntree = 5, seed = -13)
saveRDS(f, args[1])
cat("Saved ordinary survival forest to", args[1], "\n")
