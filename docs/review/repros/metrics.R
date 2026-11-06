# Run with R_LIBS_USER pointing to fresh review library, from repository root.
suppressPackageStartupMessages(library(randomForestSRC))
cat('Package:', find.package('randomForestSRC'),'\n')
e <- new.env(parent=asNamespace('randomForestSRC'))
sys.source('R/utilities_survival.R',e)
shim <- function(time,status,entry=NULL,S=NULL,grid=sort(unique(time))) {
 n<-length(time)
 if(is.null(S)) S<-matrix(rep(seq(.8,.1,length.out=length(grid)),each=n),n)
 o<-list(family='surv', yvar=cbind(time,status), predicted=rowSums(1-S),
         survival.oob=S,predicted.oob=rowSums(1-S),time.interest=grid,
         forest=list(yvar=cbind(time,status),xvar=data.frame(x=seq_len(n))),
         imputed.indv=NULL,entry.time=entry)
 class(o)<-c('rfsrc','grow','surv');o
}
# 1. Subsetting must not change a score when it only permutes all subjects.
# Outcome/prediction misalignment existed before the LT work. The new entry
# indexing retains that bug and pairs the wrong entry time with each curve.
S<-rbind(c(.9,.8,.7,.6),c(.8,.6,.4,.2),c(.6,.4,.2,.1),c(.4,.3,.2,.1))
o<-shim(c(1,2,3,4),c(1,1,0,1),c(0,.5,1,1.5),S)
a<-e$get.brier.survival(o,subset=1:4,entry.time=o$entry.time)$crps
b<-e$get.brier.survival(o,subset=4:1,entry.time=o$entry.time)$crps
cat('Subset permutation CRPS:',a,b,'\n');stopifnot(abs(a-b)>.5)
plain_a<-e$get.brier.survival(o,subset=1:4)$crps
plain_b<-e$get.brier.survival(o,subset=4:1)$crps
cat('Without LT, same inherited permutation bug:',plain_a,plain_b,'\n')
stopifnot(abs(plain_a-plain_b)>.1)
# 2. Allowed inclusive entry causes zero division at earliest tied event.
o<-shim(c(1,2,3,4),rep(1,4),rep(1,4))
b<-e$get.brier.survival(o,entry.time=o$entry.time)
cat('Tied-entry earliest case Brier row:',b$brier.matx[1,],'\n')
stopifnot(all(is.infinite(b$brier.matx[1,])))
a<-e$get.auc.survival(o,entry.time=o$entry.time)
stopifnot(is.na(a$auc$auc[1]))
# 3. Actual predict objects cannot score different-sized LT test sets;
# equal-sized test sets silently use training outcomes. The outcome overwrite
# predates LT for Brier; the new shared setup propagates it to the new AUC
# function, and the new entry validation additionally rejects test-sized L.
set.seed(42);d<-data.frame(time=rexp(40)+1,status=rep(c(0,1),20),x=rnorm(40))
f<-rfsrc(Surv(time,status)~x,d,ntree=10,entry.time=rep(.2,40))
p<-predict(f,newdata=d[1:15,])
a<-tryCatch(e$get.auc.survival(p,entry.time=rep(.2,15)),error=conditionMessage)
cat('Test-size error:',a,'\n');stopifnot(is.character(a))
test<-d;test$time<-test$time+100;test$status<-1-test$status
p<-predict(f,newdata=test)
a<-e$get.auc.survival(p,entry.time=rep(.2,40))
stopifnot(identical(as.numeric(a$event.info$time),as.numeric(d$time)),
          !identical(as.numeric(a$event.info$time),as.numeric(test$time)))
cat('Equal-sized test set scored against training times: TRUE\n')
# 4. Under independent uniform source entry/event distributions, retained
# entry ECDF is 2*l-l^2, not source entry CDF l.
# P(L<=x | L<T) = integral_0^x (1-l) dl / P(L<T)
#                = (x-x^2/2)/(1/2) = 2*x-x^2.
x <- .5
source_cdf <- x
selected_cdf <- (x-x^2/2)/.5
stopifnot(source_cdf == .5, selected_cdf == .75)
set.seed(901);N<-1000000;L<-runif(N);T<-runif(N);keep<-L<T
est<-mean(L[keep]<=.5)
cat('Selected F_L(.5):',est,';true source CDF:.5;selected analytic:.75\n')
stopifnot(abs(est-.75)<.005)
# 5. Actual LT Brier implementation scores a distorted survival curve better
# than oracle, despite no censoring and correctly independent truncation.
set.seed(12);N<-12000;L<-runif(N);T<-runif(N);keep<-L<T
L<-L[keep];T<-T[keep];grid<-seq(.01,1,.01);n<-length(T)
res<-sapply(c(1,1.3),function(alpha) {
 S<-matrix(rep((1-grid)^alpha,each=n),nrow=n)
 o<-shim(T,rep(1,n),L,S,grid)
 b<-e$get.brier.survival(o,entry.time=L)
 c(at_half=b$brier.score$brier.score[50],crps=b$crps)
})
colnames(res)<-c('oracle','distorted');print(res)
stopifnot(res[1,2]<res[1,1],res[2,2]<res[2,1])
cat('Confirmed all five metric counterexamples.\n')
