## Reproduce fix 12 timings against an isolated installed package.
## Example: R_LIBS_USER=/path/to/library Rscript docs/review/repros/fix12-concordance-timing.R /tmp/timings.csv
## Compare before/after builds on the same machine. Timing is not a unit test.
suppressPackageStartupMessages(library(randomForestSRC))
options(rf.cores=1)
args <- commandArgs(TRUE)
set.seed(42)
n <- 20000L
reps <- 3L
time <- sample(1:10000, n, replace = TRUE)
status <- rbinom(n,1,.7)
risk <- sample(1:10000, n, replace = TRUE)
w <- runif(n,.5,2)
rows <- list()
for (weighted in c(FALSE,TRUE)) {
  for (zeros in c(FALSE,TRUE)) {
    call.args <- list(time=time,censoring=status,predicted=risk,
                       entry=if(zeros) rep(0,n) else NULL)
    if (weighted) call.args$weight <- w
    elapsed <- system.time(for (i in seq_len(reps)) value <- do.call(get.cindex,call.args))[["elapsed"]]
    rows[[length(rows)+1L]] <- data.frame(weighted=weighted,entry=if(zeros)'zero' else 'NULL',n=n,repetitions=reps,
                                          seconds_per_call=elapsed/reps,score=value)
  }
}
result <- do.call(rbind,rows)
write.csv(result,args[1],row.names=FALSE)
print(result,row.names=FALSE,digits=12)
stopifnot(isTRUE(all.equal(result$score[c(1,3)],result$score[c(2,4)],tolerance=1e-8)))
