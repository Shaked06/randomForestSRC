## Integration audit for the pinned versions in UPSTREAM_SYNC_REVIEW.md.
suppressPackageStartupMessages(library(randomForestSRC))
options(rf.cores=1)
args <- commandArgs(TRUE)
set.seed(43)
n <- 90L
d <- data.frame(time=runif(n,1,10),status=rep(c(0,1,1),length.out=n),x=rnorm(n),z=rnorm(n))
entry <- runif(n,0,.9*d$time)
result <- list()
for (positive in c(FALSE,TRUE)) for (memory in c(FALSE,TRUE)) {
  set.seed(24)
  fit <- rfsrc(Surv(time,status)~.,d,ntree=35,entry.time=if(positive) entry else rep(0,n),
                save.memory=memory,nodesize=4)
  grow <- predict(fit)
  p <- predict(fit,newdata=d[1:13,])
  result[[paste(positive,memory,sep='/')]] <- list(fit=fit,
               restored=list(survival=grow$survival.oob,predicted=grow$predicted.oob),
               validation=list(survival=p$survival,predicted=p$predicted))
}
saveRDS(result,args[1])
cat('Saved',length(result),'pre-integration forests\n')
