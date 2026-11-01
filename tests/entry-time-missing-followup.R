## Regression: do not impute an exit below a subject's known entry time.
library(randomForestSRC)
options(rf.cores = 1)

fit.one.node <- function(data, entry.time = NULL, nimpute = 1) {
  rfsrc(Surv(time, status) ~ x, data, ntree = 1, bootstrap = "none",
        nodesize = 1000, entry.time = entry.time, na.action = "na.impute",
        nimpute = nimpute, seed = -13)
}
check.valid.survival <- function(fit) {
  stopifnot(all(is.finite(fit$survival)),
            all(fit$survival >= 0 & fit$survival <= 1),
            all(is.finite(fit$chf)), all(fit$chf >= 0))
}

## Exact adversarial fixture: all available time donors precede the missing
## subjects' entry. Previously this silently returned survival = -9.
d <- data.frame(time = c(rep(NA_real_, 90), rep(1, 10)), status = 1, x = 1:100)
entry <- c(rep(2, 90), rep(0, 10))
for (iterations in c(1, 3)) {
  result <- tryCatch(fit.one.node(d, entry, nimpute = iterations),
                     error = identity)
  if (!inherits(result, "error")) {
    stop("Expected an error for a missing follow-up with positive entry; got survival range ",
         paste(range(result$survival), collapse = " to "))
  }
  stopifnot(grepl("Missing follow-up times", conditionMessage(result), fixed = TRUE),
            grepl("positive entry.time", conditionMessage(result), fixed = TRUE))
}

## NaN follows the same missing-data protocol as NA.
d$time[1] <- NaN
result <- tryCatch(fit.one.node(d, entry), error = identity)
stopifnot(inherits(result, "error"),
          grepl("Missing follow-up times", conditionMessage(result), fixed = TRUE))

## The restriction must not change ordinary missing-response imputation.
for (iterations in c(1, 3)) {
  ordinary <- fit.one.node(d, nimpute = iterations)
  zero <- fit.one.node(d, rep(0, nrow(d)), nimpute = iterations)
  check.valid.survival(ordinary)
  check.valid.survival(zero)
  stopifnot(identical(ordinary$survival, zero$survival),
            identical(ordinary$chf, zero$chf),
            identical(ordinary$imputed.data, zero$imputed.data))
}

## Known LT responses still support missing predictors and missing status.
complete <- data.frame(time = 1:12, status = 1, x = 1:12)
predictor.missing <- complete
predictor.missing$x[1] <- NA_real_
status.missing <- complete
status.missing$status[1] <- NA_real_
check.valid.survival(fit.one.node(complete, rep(.5, 12)))
check.valid.survival(fit.one.node(predictor.missing, rep(.5, 12)))
check.valid.survival(fit.one.node(status.missing, rep(.5, 12)))

## A missing time on a zero-entry subject is safe, even when other subjects
## have delayed entry. The guard should be per subject, not dataset-wide.
time.missing <- complete
time.missing$time[1] <- NA_real_
mixed.entry <- c(0, rep(.5, 11))
check.valid.survival(fit.one.node(time.missing, mixed.entry))

## Restored forests need the same checks. Simulate legacy stored responses
## containing a missing exit, or an invalid exit already filled in by imputation.
stored <- fit.one.node(complete, rep(.5, 12))
missing <- stored
missing$forest$yvar[1, 1] <- NA_real_
result <- tryCatch(predict(missing), error = identity)
stopifnot(inherits(result, "error"),
          grepl("Missing follow-up times", conditionMessage(result), fixed = TRUE))
result <- tryCatch(partial(missing, partial.type = "surv", partial.xvar = "x",
                            partial.values = 1, partial.time = 1), error = identity)
stopifnot(inherits(result, "error"),
          grepl("Missing follow-up times", conditionMessage(result), fixed = TRUE))

invalid <- stored$forest
invalid$yvar[1, 1] <- .25
result <- tryCatch(predict(invalid, newdata = complete["x"]), error = identity)
stopifnot(inherits(result, "error"),
          grepl("entry.time must not exceed", conditionMessage(result), fixed = TRUE))
