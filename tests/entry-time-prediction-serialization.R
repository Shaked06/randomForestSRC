## Regression: a saved forest must predict in a process that never grew one.
library(randomForestSRC)
options(rf.cores = 1)

test.serialization <- function() {
  work <- tempfile("entry-time-serialization-")
  dir.create(work)
  on.exit(unlink(work, recursive = TRUE), add = TRUE)
  child <- file.path(work, "predict.R")
  writeLines(c(
    "args <- commandArgs(trailingOnly = TRUE)",
    "fixture <- readRDS(args[1])",
    ".libPaths(fixture$lib.paths)",
    "library(randomForestSRC)",
    "options(rf.cores = 1)",
    "fit <- fixture$fit",
    "check <- function(p) {",
    "  stopifnot(isTRUE(all.equal(p$survival, fit$survival, tolerance=1e-12)),",
    "            isTRUE(all.equal(p$chf, fit$chf, tolerance=1e-12)))",
    "}",
    "check(predict(fit, newdata=fixture$newdata))",
    "check(predict(fit$forest, newdata=fixture$newdata))",
    "check(predict(fit))",
    "pd <- partial(fit, oob=TRUE, partial.type='surv', partial.xvar='x',",
    "              partial.values=c(-1,0,1), partial.time=max(fit$time.interest), seed=-13)",
    "stopifnot(!is.null(pd$survOutput),",
    "          isTRUE(all.equal(pd$survOutput, fixture$partial.survival, tolerance=1e-12)))"
  ), child)

  set.seed(18)
  n <- 80L
  d <- data.frame(time = runif(n, 1, 10), status = rbinom(n, 1, .7),
                  x = rnorm(n))
  for (save.memory in c(FALSE, TRUE)) {
    for (left.truncated in c(FALSE, TRUE)) {
      fit <- rfsrc(Surv(time, status) ~ x, d, ntree = 5,
                   entry.time = if (left.truncated) .8 * d$time else NULL,
                   save.memory = save.memory, seed = -13)
      pd <- partial(fit, oob = TRUE, partial.type = "surv", partial.xvar = "x",
                    partial.values = c(-1, 0, 1),
                    partial.time = max(fit$time.interest), seed = -13)
      fixture <- file.path(work, "fixture.rds")
      saveRDS(list(fit = fit, newdata = d["x"], lib.paths = .libPaths(),
                   partial.survival = pd$survOutput), fixture)
      log <- file.path(work, "child.log")
      status <- suppressWarnings(system2(file.path(R.home("bin"), "Rscript"),
                                          c("--vanilla", shQuote(child), shQuote(fixture)),
                                          stdout = log, stderr = log))
      if (status != 0L) {
        stop(paste("Saved-model prediction failed:",
                   paste(readLines(log, warn = FALSE), collapse = "\n")))
      }
    }
  }
}
test.serialization()
