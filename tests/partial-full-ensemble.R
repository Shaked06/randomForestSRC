## Partial predictions must average over the trees requested by oob.
## Full-ensemble references use ordinary prediction on modified covariates;
## OOB references explicitly average each subject's eligible tree predictions.
library(randomForestSRC)
options(rf.cores = 1)

test.partial.ensembles <- function() {
  work <- tempfile("partial-ensemble-"); dir.create(work)
  on.exit(unlink(work, recursive = TRUE), add = TRUE)
  set.seed(15)
  n <- 72L
  d <- data.frame(time = runif(n, 1, 10), status = rep(c(0, 1, 1), length.out = n),
                    y = rnorm(n), g = factor(rep(c("a", "b", "c"), length.out = n)),
                    x = rnorm(n), z = rnorm(n))
  samp <- vapply(1:7, function(b) as.integer((seq_len(n) + 11*b) %% n < 45), integer(n))
  stopifnot(all(rowSums(samp == 0) > 0))
  fixtures <- list()
  for (memory in c(FALSE, TRUE)) {
    for (kind in c("ordinary", "zero", "LT", "regression", "classification", "mixed", "CR")) {
      data <- d
      formula <- switch(kind, regression = y ~ x + z, classification = g ~ x + z,
                          mixed = Multivar(y, g) ~ x + z, Surv(time, status) ~ x + z)
      if (kind == "CR") data$status <- rep(0:2, length.out = n)
      entries <- switch(kind, zero = rep(0, n), LT = .8*data$time, NULL)
      fit <- rfsrc(formula, data, ntree = 7, nodesize = 4, seed = -13,
                    bootstrap = "by.user", samp = samp, entry.time = entries,
                    save.memory = memory)
      fixtures[[paste(kind, memory)]] <- list(fit = fit, data = data[c("x", "z")], samp = samp)
    }
  }

  check.fixture <- function(fixture) {
    fit <- fixture$fit
    n <- nrow(fixture$data)
    values <- c(-1, 0, 1)
    times <- if (grepl("surv", fit$family)) {
      unique(fit$time.interest[c(1, ceiling(length(fit$time.interest)/2), length(fit$time.interest))])
    } else NULL
    time.index <- match(times, fit$time.interest)
    types <- if (fit$family == "surv") c("surv", "chf", "mort", "rel.freq") else
      if (fit$family == "surv-CR") c("cif", "chf", "years.lost") else "response"
    same <- function(x, y) isTRUE(all.equal(as.numeric(x), as.numeric(y), tolerance = 1e-12))
    components <- function(object, type, partial = FALSE, grid = NULL) {
      if (grepl("surv", fit$family)) {
        if (partial) {
          x <- object$survOutput
          idx <- c(list(x), rep(list(TRUE), length(dim(x)) - 1L), list(grid, drop = FALSE))
          return(list(survival = as.numeric(do.call(`[`, idx))))
        }
        x <- switch(type, surv = object$survival[, time.index, drop = FALSE],
                     chf = if (fit$family == "surv") object$chf[, time.index, drop = FALSE] else
                       object$chf[, time.index, , drop = FALSE],
                     cif = object$cif[, time.index, , drop = FALSE], object$predicted)
        return(list(survival = as.numeric(x)))
      }
      if (partial) {
        return(c(lapply(object$regrOutput, function(x) as.numeric(x[, grid])),
                 lapply(object$classOutput, function(x) as.numeric(x[, -1, grid, drop = FALSE]))))
      }
      if (fit$family %in% c("regr", "class")) return(list(as.numeric(object$predicted)))
      c(lapply(object$regrOutput, function(x) as.numeric(x$predicted)),
        lapply(object$classOutput, function(x) as.numeric(x$predicted)))
    }
    partial.call <- function(type, trees = NULL, oob = FALSE, object = fit, jitt = FALSE) {
      partial(object, oob = oob, partial.xvar = "x", partial.values = values,
                partial.xvar2 = "z", partial.values2 = .25,
                partial.type = if (type == "response") NULL else type,
                partial.time = times, get.tree = trees, seed = -13, jitt = jitt)
    }
    ## The first native call in the child is full-ensemble partial(), before
    ## any grow/restore/OOB operation could leave a count array behind.
    first <- partial.call(types[1])
    for (trees in list(NULL, c(2, 4, 6), 4L)) {
      predictions <- lapply(values, function(value) {
        data <- fixture$data; data$x <- value; data$z <- .25
        predict(fit, newdata = data, get.tree = trees, seed = -13)
      })
      for (type in types) {
        actual <- if (is.null(trees) && type == types[1]) first else partial.call(type, trees)
        for (k in seq_along(values)) {
          observed <- components(actual, type, partial = TRUE, grid = k)
          expected <- components(predictions[[k]], type)
          stopifnot(length(observed) == length(expected))
          for (j in seq_along(expected)) stopifnot(same(observed[[j]], expected[[j]]))
        }
      }
    }
    ## Check the OOB path independently, using row-specific eligibility.
    per.tree <- lapply(values, function(value) {
      data <- fixture$data; data$x <- value; data$z <- .25
      lapply(1:7, function(tree) predict(fit, newdata = data, get.tree = tree, seed = -13))
    })
    for (type in types) {
      actual <- partial.call(type, oob = TRUE)
      for (k in seq_along(values)) {
        per.component <- lapply(per.tree[[k]], components, type = type)
        observed <- components(actual, type, partial = TRUE, grid = k)
        for (j in seq_along(observed)) {
          numerator <- rep(0, length(observed[[j]]))
          for (tree in 1:7) numerator <- numerator + per.component[[tree]][[j]] *
            rep(fixture$samp[, tree] == 0, length.out = length(numerator))
          expected <- numerator / rep(rowSums(fixture$samp == 0), length.out = length(numerator))
          stopifnot(same(observed[[j]], expected))
        }
      }
    }
    ## Full after OOB, direct forest input, and alternate tree restoration
    ## must all use the same full-ensemble denominator.
    for (jit in c(FALSE, TRUE)) {
      repeated <- partial.call(types[1], object = fit$forest, jitt = jit)
      stopifnot(same(unlist(components(first, types[1], TRUE, 1L)),
                     unlist(components(repeated, types[1], TRUE, 1L))))
    }
  }
  fixture.path <- file.path(work, "fixtures.rds")
  saveRDS(list(fixtures = fixtures, check = check.fixture, libraries = .libPaths()), fixture.path)
  child <- file.path(work, "check.R")
  writeLines(c("args <- commandArgs(TRUE)", "fixture <- readRDS(args[1])",
               ".libPaths(fixture$libraries)", "library(randomForestSRC)",
               "options(rf.cores = 1)",
               "for (name in names(fixture$fixtures)) {",
               "  fixture$check(fixture$fixtures[[name]])",
               "  cat(name, 'passed\\n')", "}"), child)
  log <- file.path(work, "child.log")
  status <- suppressWarnings(system2(file.path(R.home("bin"), "Rscript"),
                                      c("--vanilla", shQuote(child), shQuote(fixture.path)),
                                      stdout = log, stderr = log))
  if (status != 0L) stop("Partial ensemble regression failed in child:\n",
                         paste(readLines(log, warn = FALSE), collapse = "\n"))
}
test.partial.ensembles()
