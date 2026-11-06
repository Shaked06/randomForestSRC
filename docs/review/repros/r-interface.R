## Run from repository root, optionally selecting a library via R_LIBS:
## R_LIBS=/path/to/review/library Rscript docs/review/repros/r-interface.R
## These assertions characterize observed bugs; they are not desired behavior.
suppressPackageStartupMessages(library(randomForestSRC))
options(rf.cores = 1, mc.cores = 1)
set.seed(6)
n <- 120L
d <- data.frame(time = rexp(n) + 1, status = rbinom(n, 1, .7),
                x = rnorm(n), z = rnorm(n))
entry <- runif(n) * d$time * .95
message_or_ok <- function(expr) {
  tryCatch({ force(expr); "OK" }, error = conditionMessage)
}

## BUG: default NA omission changes data rows without subsetting entry.time.
d.na <- d
d.na$x[2] <- NA_real_
baseline <- rfsrc(Surv(time, status) ~ ., d.na, ntree = 10)
na.error <- message_or_ok(rfsrc(Surv(time, status) ~ ., d.na, ntree = 10,
                               entry.time = entry))
stopifnot(baseline$n == n - 1L, grepl("Invalid entry.time", na.error))
cat("BUG NA omission: ordinary fit retains", baseline$n,
    "rows; entry-aware fit rejects the original-length entry vector.\n")

## BUG: restore-mode performance and standalone VIMP silently score entry-blind.
## Keep grow and restore in the same R process: fresh-process restoration has
## an additional native crash covered by the C review's reproduction.
f <- rfsrc(Surv(time, status) ~ ., d, ntree = 100, entry.time = entry,
           use.uno = FALSE, importance = FALSE, seed = -22)
p <- predict(f)
v <- vimp(f, importance = "permute", seed = -33)
blind <- get.cindex(d$time, d$status, f$predicted.oob)
aware <- get.cindex(d$time, d$status, f$predicted.oob, entry = entry)
grow.error <- unname(tail(f$err.rate, 1))
restore.error <- unname(tail(p$err.rate, 1))
vimp.error <- unname(tail(v$err.rate, 1))
stopifnot(identical(f$predicted.oob, p$predicted.oob),
          isTRUE(all.equal(grow.error, aware)),
          isTRUE(all.equal(restore.error, blind)),
          isTRUE(all.equal(vimp.error, blind)), abs(blind - aware) > 1e-4)
cat(sprintf(paste0("BUG restored scoring: unchanged OOB predictions, ",
                   "grow=%.7f, predict=%.7f, vimp=%.7f; ",
                   "aware=%.7f, blind=%.7f.\n"),
            grow.error, restore.error, vimp.error, aware, blind))

## BUG: tuning holdout subsets observations but leaves entry.time unchanged.
args <- list(formula = Surv(time, status) ~ ., data = d, sampsize = 30,
             ntree.try = 20, nodesize.try = 5, mtry.start = 1,
             max.iter = 1, do.best = FALSE, seed = 1)
tune.baseline <- message_or_ok(do.call(tune, args))
tune.entry <- message_or_ok(do.call(tune, c(args, list(entry.time = entry))))
stopifnot(tune.baseline == "OK", grepl("Invalid entry.time", tune.entry))
cat("BUG tuning holdout: ordinary tuning succeeds; entry-aware tuning fails length validation.\n")

## BUG: unsupported split rules accept delayed entry without a guard.
for (split in c("logrankscore", "bs.gradient")) {
  warnings <- character()
  fit <- withCallingHandlers(
    rfsrc(Surv(time, status) ~ ., d, ntree = 10,
          entry.time = entry, splitrule = split),
    warning = function(w) {
      warnings <<- c(warnings, conditionMessage(w))
      invokeRestart("muffleWarning")
    })
  stopifnot(fit$splitrule == split, length(warnings) == 0L)
  cat("BUG unsupported split:", split, "accepted nonzero entry.time without warning.\n")
}
