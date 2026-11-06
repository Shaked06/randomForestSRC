## Regression: entry times follow the exact rows retained by preprocessing.
library(randomForestSRC)
options(rf.cores = 1)
same <- function(x, y) isTRUE(all.equal(x, y, tolerance = 1e-12))
grow <- function(data, entry = NULL, na.action = "na.omit", memory = FALSE,
                  fast = FALSE, formula = Surv(time, status) ~ x + z) {
  set.seed(22)
  engine <- if (fast) rfsrc.fast else rfsrc
  engine(formula, data, entry.time = entry, na.action = na.action,
          ntree = 12, nodesize = 4, seed = -61, save.memory = memory)
}
check.omission <- function(data, entry, keep, na.action = "na.omit",
                            memory = FALSE, fast = FALSE,
                            formula = Surv(time, status) ~ x + z) {
  actual <- grow(data, entry, na.action, memory, fast, formula)
  reference <- grow(data[keep, , drop = FALSE], entry[keep],
                     na.action, memory, fast, formula)
  stopifnot(actual$n == length(keep),
            identical(actual$entry.time, entry[keep]))
  for (field in c("predicted", "predicted.oob", "survival", "survival.oob",
                  "chf", "chf.oob", "err.rate", "yvar", "xvar")) {
    stopifnot(same(actual[[field]], reference[[field]]))
  }
  if (isTRUE(actual$forest$forest)) {
    stopifnot(identical(actual$forest$entry.time, entry[keep]))
    restored <- predict(actual)
    expected <- predict(reference)
    stopifnot(same(restored$survival.oob, expected$survival.oob),
              same(restored$err.rate, expected$err.rate))
  }
  invisible(actual)
}
expect.error <- function(expr, pattern = "Invalid entry.time") {
  error <- tryCatch(force(expr), error = identity)
  stopifnot(inherits(error, "error"), grepl(pattern, conditionMessage(error)))
}

set.seed(35)
n <- 80L
d <- data.frame(time = runif(n, 1, 12), status = rep(c(1, 1, 0), length.out = n),
                 x = rnorm(n), z = rnorm(n))
rownames(d) <- paste0("subject-", sample(1000:9000, n))
entry <- .8*d$time
entry[1:25] <- .05
## Names on entry values do not override positional alignment with data rows.
names(entry) <- paste0("entry-", sample(1000:9000, n))

## Omit first, middle, and final rows for missing predictors or responses.
missing <- d
missing$x[1] <- NA_real_
missing$z[19] <- NaN
missing$time[40] <- NA_real_
missing$status[n] <- NA_real_
keep <- setdiff(seq_len(n), c(1, 19, 40, n))
for (memory in c(FALSE, TRUE)) {
  check.omission(missing, entry, keep, memory = memory)
}
check.omission(as.matrix(missing), entry, keep)

## Entirely empty rows are removed by preliminary cleanup, including in
## imputation mode; partially missing predictor rows are retained there.
empty <- d
empty[5, ] <- NA_real_
empty$x[9] <- NA_real_
check.omission(empty, entry, setdiff(seq_len(n), c(5, 9)))
check.omission(empty, entry, setdiff(seq_len(n), 5), na.action = "na.impute")

## Factor level cleanup converts a literal "NA" level into missing data.
## Follow the package's actual retained rows, rather than complete.cases()
## on the original input before that conversion.
factor.data <- d
factor.data$f <- factor(rep(c("A", "B", "NA"), length.out = n))
factor.data$x[2] <- NA_real_
keep.factor <- setdiff(which(as.character(factor.data$f) != "NA"), 2)
check.omission(factor.data, entry, keep.factor,
                formula = Surv(time, status) ~ x + f)

## Missing values outside the formula must not remove extra observations.
unused <- d
unused$unused <- rep(c(NA_real_, 1), length.out = n)
check.omission(unused, entry, seq_len(n))

## Omitted subjects' entry values are also omitted before value validation.
missing.entry <- entry
missing.entry[1] <- NA_real_
check.omission(missing, missing.entry, keep)

## No additional row omission in normal predictor-imputation mode.
impute <- d
impute$x[c(2, 21)] <- NA_real_
fit <- check.omission(impute, entry, seq_len(n), na.action = "na.impute")
stopifnot(fit$n == n, !is.null(fit$imputed.indv))

## NULL keeps ordinary behavior; explicit zeros follow the retained rows.
ordinary <- grow(missing)
zero <- grow(missing, rep(0, n))
stopifnot(is.null(ordinary$entry.time), is.null(ordinary$forest$entry.time),
          identical(zero$entry.time, rep(0, length(keep))),
          same(ordinary$survival, zero$survival),
          same(ordinary$err.rate, zero$err.rate))

## The public fast wrapper must use the same grow-time row alignment.
check.omission(missing, entry, keep, fast = TRUE)

## Reject a pre-filtered or incorrectly sized vector supplied alongside
## unfiltered data. Retained entries must still satisfy the normal guards.
expect.error(grow(missing, entry[keep]), "original input data")
expect.error(grow(d, entry[-1]), "original input data")
for (value in c(NA_real_, -1, Inf)) {
  invalid <- entry
  invalid[3] <- value
  expect.error(grow(missing, invalid))
}
invalid <- entry
invalid[3] <- d$time[3] + 1
expect.error(grow(missing, invalid), "must not exceed")
expect.error(grow(d, as.character(entry)))
