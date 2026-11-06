## Integration audit for the pinned versions in UPSTREAM_SYNC_REVIEW.md.
suppressPackageStartupMessages(library(randomForestSRC))
options(rf.cores = 1)
args <- commandArgs(TRUE)
outfile <- args[1]
with.zero <- identical(args[2], 'zero')
set.seed(88)
n <- 90L
x <- rnorm(n)
z <- runif(n)
surv <- data.frame(time = exp(.3*x + rnorm(n)) + 1,
                    status = rep(c(0,1,1),length.out=n), x=x,z=z)
reg <- data.frame(y = 2*x + z + rnorm(n), x=x,z=z)
clas <- data.frame(y=factor(rep(c('A','B','C'),length.out=n)),x=x,z=z)
surv$time[which.max(surv$time)] <- max(surv$time)+1
surv$status[which.max(surv$time)] <- 1
collect <- function(o) lapply(setNames(c('predicted','predicted.oob','survival',
     'survival.oob','chf','chf.oob','err.rate','importance'),
     c('predicted','predicted.oob','survival','survival.oob','chf','chf.oob','err.rate','importance')),
     function(key) o[[key]])
answer <- list()
for (protocol in c('3.10.0','3.2.3')) {
  rfsrc.RNGversion(protocol)
  for (family in c('survival','regression','classification')) {
    data <- switch(family,survival=surv,regression=reg,classification=clas)
    formula <- if (family=='survival') Surv(time,status)~x+z else y~x+z
    for (missing in c(FALSE,TRUE)) {
      if (missing) data$x[c(2,7)] <- NA
      extra <- if(with.zero && family=='survival') list(entry.time=rep(0,n)) else list()
      set.seed(12)
      fit <- do.call(rfsrc,c(list(formula=formula,data=data,ntree=35,nodesize=4,
                                  ntime=12,na.action='na.impute',importance='anti'),extra))
      p <- predict(fit,newdata=data[1:20,])
      restored <- predict(fit)
      key <- paste(protocol,family,missing,sep='/')
      answer[[key]] <- list(grow=collect(fit),predict=collect(p),restore=collect(restored),
                            rng=fit$forest$rng.version)
      if (family=='survival') {
        answer[[key]]$brier <- get.brier.survival(p)$brier.score
        answer[[key]]$auc <- get.auct.survival(p)$auct.score
      }
      ## Saved protocol governs restoration even after the session changes.
      rfsrc.RNGversion(if (protocol=='3.10.0') '3.2.3' else '3.10.0')
      again <- predict(fit)
      stopifnot(isTRUE(all.equal(collect(restored),collect(again),tolerance=1e-12)))
      rfsrc.RNGversion(protocol)
    }
  }
}
saveRDS(answer,outfile)
cat(length(answer),'profiles saved to',outfile,'\n')
