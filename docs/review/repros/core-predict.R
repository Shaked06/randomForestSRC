## Run in a fresh R process after core-grow.R. On the reviewed source this
## terminates R with a segmentation fault. Capture stderr to a log because
## R's native crash handler prints a large traceback.
suppressPackageStartupMessages(library(randomForestSRC))
options(rf.cores = 1)
args <- commandArgs(trailingOnly = TRUE)
stopifnot(length(args) == 1L)
f <- readRDS(args[1])
cat("Loaded saved forest; predicting in a fresh R process\n")
flush.console()
p <- predict(f, newdata = data.frame(x = 0))
stopifnot(all(is.finite(p$survival)))
cat("Prediction completed without a native crash\n")
