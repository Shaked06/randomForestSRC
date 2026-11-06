## Historical provenance check for bug 7, independent of subsequent local fixes.
## Pass the directory containing the two pinned source files described in
## docs/review/BUG7_PROVENANCE.md. Run with the intended local package library.
args <- commandArgs(trailingOnly = TRUE)
if (length(args) != 1L) stop("Supply the directory containing the pinned upstream and pre-LT R sources")
source.dir <- args[1L]
suppressPackageStartupMessages(library(randomForestSRC))
options(rf.cores = 1)
old <- new.env(parent = asNamespace('randomForestSRC'))
upstream <- new.env(parent = asNamespace('randomForestSRC'))
sys.source(file.path(source.dir, 'pre-lt-utilities_survival.R'), old)
sys.source(file.path(source.dir, 'utilities_survival.R'), upstream)
set.seed(763)
n <- 40L
train <- data.frame(time = runif(n, 1, 6), status = 1, x = rnorm(n))
fit <- rfsrc(Surv(time, status) ~ x, train, ntree = 40, ntime = 12, nodesize = 3)
test.A <- data.frame(time = train$time + 100, status = 1, x = rnorm(n))
test.B <- test.A
test.B$time <- .01 + .09*(max(train$time) - train$time)/diff(range(train$time))
pA <- predict(fit, newdata = test.A)
pB <- predict(fit, newdata = test.B)
stopifnot(identical(pA$survival, pB$survival),
          isTRUE(all.equal(pA$yvar[,1], test.A$time)),
          isTRUE(all.equal(pB$yvar[,1], test.B$time)))
S <- pA$survival
grid <- pA$time.interest
truth.A <- colMeans((1 - S)^2)
truth.B <- colMeans(S^2)
training <- colMeans((outer(train$time, grid, '>') - S)^2)
results <- list()
for (version in c('pre_LT_snapshot','local_LT_branch','upstream_30d95e2')) {
  scorer <- switch(version, pre_LT_snapshot = old$get.brier.survival,
                     local_LT_branch = randomForestSRC::get.brier.survival,
                     upstream_30d95e2 = upstream$get.brier.survival)
  a <- scorer(pA)$brier.score$brier.score
  b <- scorer(pB)$brier.score$brier.score
  results[[version]] <- data.frame(version = version,
      changed_when_test_outcomes_changed = !isTRUE(all.equal(a,b)),
      A_matches_test_outcomes = isTRUE(all.equal(a,truth.A)),
      B_matches_test_outcomes = isTRUE(all.equal(b,truth.B)),
      A_matches_training_outcomes = isTRUE(all.equal(a,training)),
      mean_score_A = mean(a), mean_score_B = mean(b))
}
result <- do.call(rbind,results)
rownames(result) <- NULL
print(result, row.names = FALSE)
stopifnot(!result$changed_when_test_outcomes_changed[1],
          result$A_matches_training_outcomes[1],
          result$A_matches_test_outcomes[3], result$B_matches_test_outcomes[3])
## Native prediction error uses the supplied new-data outcomes, separately
## from the R Brier helper. Check it against literal pair counting.
error.reference <- function(time, risk) {
  pairs <- combn(seq_along(time),2)
  early <- ifelse(time[pairs[1,]] < time[pairs[2,]], pairs[1,], pairs[2,])
  late <- ifelse(time[pairs[1,]] < time[pairs[2,]], pairs[2,], pairs[1,])
  mean((risk[early] < risk[late]) + .5*(risk[early] == risk[late]))
}
errors <- c(native.A = tail(as.numeric(pA$err.rate),1),
            reference.A = error.reference(test.A$time, pA$predicted),
            native.B = tail(as.numeric(pB$err.rate),1),
            reference.B = error.reference(test.B$time, pB$predicted))
print(errors)
stopifnot(isTRUE(all.equal(unname(errors[1]),unname(errors[2]))),
          isTRUE(all.equal(unname(errors[3]),unname(errors[4]))))
pNo <- predict(fit,newdata=test.A['x'])
err <- tryCatch(upstream$get.brier.survival(pNo),error = identity)
stopifnot(inherits(err,'error'),grepl('evaluation outcomes are required',conditionMessage(err)))
cat('Upstream unlabeled-data behavior:',conditionMessage(err),'\n')
write.csv(result,file.path(source.dir, 'provenance-results.csv'),row.names=FALSE)
cat('PASS: pre-LT snapshot defect and corrected upstream scorer; see local row for local status.\n')
