## Integration audit for the pinned versions in UPSTREAM_SYNC_REVIEW.md.
suppressPackageStartupMessages(library(randomForestSRC))
options(rf.cores=1)
args <- commandArgs(TRUE)
models <- readRDS(args[1])
for (key in names(models)) {
  case <- models[[key]]
  stopifnot(is.null(case$fit$forest$rng.version))
  rfsrc.RNGversion('3.10.0')
  restored <- predict(case$fit)
  p <- predict(case$fit,newdata=data.frame(case$fit$yvar,case$fit$xvar)[1:13,])
  stopifnot(isTRUE(all.equal(case$restored,
              list(survival=restored$survival.oob,predicted=restored$predicted.oob),tolerance=1e-12)),
            isTRUE(all.equal(case$validation,
              list(survival=p$survival,predicted=p$predicted),tolerance=1e-12)))
  cat(key,'PASS\n')
}
