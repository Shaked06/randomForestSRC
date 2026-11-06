## Integration audit for the pinned versions in UPSTREAM_SYNC_REVIEW.md.
suppressPackageStartupMessages(library(randomForestSRC))
options(rf.cores=1,mc.cores=1)
set.seed(6)
n <- 120L
d <- data.frame(time=rexp(n)+1,status=rbinom(n,1,.7),x=rnorm(n),z=rnorm(n))
entry <- runif(n)*d$time*.95
attempt <- function(expr) tryCatch({force(expr);'OK'},error=conditionMessage)
d.na <- d;d.na$x[2] <- NA_real_
baseline <- rfsrc(Surv(time,status)~.,d.na,ntree=10)
na.error <- attempt(rfsrc(Surv(time,status)~.,d.na,ntree=10,entry.time=entry))
stopifnot(baseline$n==n-1L,grepl('Invalid entry.time',na.error))
cat('10 still open:',na.error,'\n')
args <- list(formula=Surv(time,status)~.,data=d,sampsize=30,ntree.try=20,
             nodesize.try=5,mtry.start=1,max.iter=1,do.best=FALSE,seed=1)
ordinary <- attempt(do.call(tune,args))
lt <- attempt(do.call(tune,c(args,list(entry.time=entry))))
stopifnot(ordinary=='OK',grepl('Invalid entry.time',lt))
cat('11 still open:',lt,'\n')
for (split in c('logrankscore','bs.gradient')) {
  warnings <- character()
  fit <- withCallingHandlers(rfsrc(Surv(time,status)~.,d,ntree=10,
                  entry.time=entry,splitrule=split),warning=function(w) {
                    warnings <<- c(warnings,conditionMessage(w));invokeRestart('muffleWarning')})
  stopifnot(fit$splitrule==split,!length(warnings))
  cat('13 still open: accepts positive entries with',split,'without warning\n')
}
time <- rexp(300); status <- rbinom(300,1,.7); risk <- rnorm(300)
stopifnot(isTRUE(all.equal(get.cindex(time,status,risk),get.cindex(time,status,risk,entry=rep(0,300)))))
cat('12: zero/NULL concordance values agree; source still excludes non-NULL entry from Fenwick path\n')
