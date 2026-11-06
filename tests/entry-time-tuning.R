## Tuning keeps entry times attached to train/holdout subjects and scores
## held-out survival pairs using those subjects' entry times.
library(randomForestSRC)
options(rf.cores = 1)
same <- function(x, y) isTRUE(all.equal(unname(x), unname(y), tolerance = 1e-12))

check.tuning <- function() {
  ns <- asNamespace("randomForestSRC")
  audit <- new.env(parent = emptyenv())
  audit$grow <- audit$prediction <- list()
  assign(".rf.entry.tuning.audit", audit, envir = .GlobalEnv)
  trace("rfsrc", where = ns, print = FALSE, tracer = quote({
    a <- get(".rf.entry.tuning.audit", envir = .GlobalEnv)
    a$grow[[length(a$grow) + 1L]] <- list(data = as.data.frame(data),
       entry = entry.time, stump = isTRUE(nodedepth == 0))
  }))
  trace("predict.rfsrc", where = ns, print = FALSE, exit = quote({
    if (!missing(newdata)) {
      a <- get(".rf.entry.tuning.audit", envir = .GlobalEnv)
      p <- returnValue()
      a$prediction[[length(a$prediction) + 1L]] <- list(
        time = p$yvar[, 1], status = p$yvar[, 2], risk = p$predicted,
        weight = p$uno.weights, data = newdata,
        train.z = object$xvar[, "z"], train.entry = object$entry.time,
        mtry = object$mtry, nodesize = object$nodesize)
    }
  }))
  on.exit({
    untrace("rfsrc", where = ns)
    untrace("predict.rfsrc", where = ns)
    rm(".rf.entry.tuning.audit", envir = .GlobalEnv)
  }, add = TRUE)
  set.seed(6)
  n <- 120L
  d <- data.frame(time = rexp(n) + 1, status = rbinom(n, 1, .7),
                   x = rnorm(n), z = rnorm(n))
  entry <- runif(n)*d$time*.95
  rownames(d) <- paste0("subject-", sample(1000:9000, n))
  ## Independent pair enumeration, using the scorer's supplied IPCW values.
  ## Continuous follow-up times avoid event/censor ties in these fixtures.
  pair.error <- function(p, entry) {
    pairs <- combn(seq_along(p$time), 2)
    early <- ifelse(p$time[pairs[1, ]] < p$time[pairs[2, ]], pairs[1, ], pairs[2, ])
    late <- ifelse(p$time[pairs[1, ]] < p$time[pairs[2, ]], pairs[2, ], pairs[1, ])
    weight <- if (is.null(p$weight)) rep(1, length(p$time)) else p$weight
    ok <- p$status[early] == 1 & entry[late] <= p$time[early] &
      is.finite(p$risk[early]) & is.finite(p$risk[late]) &
      weight[early] > 0 & weight[late] > 0
    if (!any(ok)) return(NA_real_)
    difference <- p$risk[early[ok]] - p$risk[late[ok]]
    loss <- (difference < -1e-9) + .5*(abs(difference) <= 1e-9)
    sum(weight[early[ok]]*loss)/sum(weight[early[ok]])
  }
  saw.entry.effect <- FALSE
  run <- function(kind, method = "grid", holdout = TRUE, uno = FALSE,
                    data = d, entries = entry, best = FALSE, verify = TRUE) {
    audit$grow <- audit$prediction <- list()
    args <- list(formula = Surv(time, status) ~ x + z, data = data,
                 nodesize.try = c(3, 5), ntree.try = 8,
                 sampsize = if (holdout) 25 else 60, seed = 1,
                 nsplit = 1, method = method, final.window = 2,
                 reps.initial = 1, reps.final = 1, max.iter = 1,
                 trace = FALSE, use.uno = uno, entry.time = entries)
    if (kind == "both") {
      args$mtry.start <- 1
      args$do.best <- best
    }
    prior.seed <- .Random.seed
    result <- do.call(if (kind == "both") tune else tune.nodesize, args)
    stopifnot(identical(prior.seed, .Random.seed))
    table <- if (kind == "both") as.data.frame(result$results) else result$err
    stopifnot(nrow(table) > 0, all(is.finite(table$err)))
    if (verify) {
      ## Inspect actual grow calls, not a reconstruction of tuner internals.
      for (g in audit$grow) {
        if (g$stump) {
          stopifnot(same(g$entry, entries))
        } else {
          index <- match(g$data$z, data$z)
          stopifnot(!anyNA(index), same(g$entry, entries[index]))
        }
      }
      if (holdout) {
        stopifnot(length(audit$prediction) > 0L)
        reference <- vapply(audit$prediction, function(p) {
          index <- match(p$data$z, data$z)
          train <- match(p$train.z, data$z)
          stopifnot(!anyNA(index), !anyNA(train),
                    !length(intersect(index, train)),
                    same(p$train.entry, entries[train]))
          aware <- pair.error(p, entries[index])
          blind <- pair.error(p, rep(0, length(index)))
          if (is.finite(aware) && is.finite(blind) && abs(aware - blind) > 1e-6) {
            saw.entry.effect <<- TRUE
          }
          aware
        }, numeric(1))
        for (i in seq_len(nrow(table))) {
          matches <- vapply(audit$prediction, function(p) {
            p$nodesize == table$nodesize[i] &&
              (kind != "both" || p$mtry == table$mtry[i])
          }, logical(1))
          stopifnot(any(matches), same(table$err[i], mean(reference[matches])))
        }
      } else {
        stopifnot(!length(audit$prediction))
      }
      if (best) {
        keep <- complete.cases(data[c("time", "status", "x", "z")])
        stopifnot(result$rf$n == sum(keep),
                  same(result$rf$entry.time, entries[keep]),
                  same(result$rf$forest$entry.time, entries[keep]))
      }
    }
    result
  }
  ## Both tuners and search methods, with and without Uno weighting.
  for (kind in c("both", "nodesize")) for (method in c("grid", "golden")) {
    for (uno in c(FALSE, TRUE)) run(kind, method, uno = uno)
  }
  stopifnot(saw.entry.effect)
  ## Stump cleanup must align entries before either sampling strategy.
  missing <- d
  missing$x[2] <- NA_real_
  missing$time[5] <- NA_real_
  for (kind in c("both", "nodesize")) for (holdout in c(FALSE, TRUE)) {
    run(kind, holdout = holdout, data = missing, best = kind == "both")
  }
  ## OOB golden search uses the complete, aligned training vector as well.
  for (kind in c("both", "nodesize")) run(kind, "golden", holdout = FALSE)
  ## Zero entry reproduces ordinary tuning, including the selected settings.
  for (kind in c("both", "nodesize")) for (holdout in c(FALSE, TRUE)) {
    ordinary <- run(kind, holdout = holdout, entries = NULL, verify = FALSE)
    zero <- run(kind, holdout = holdout, entries = rep(0, n), verify = FALSE)
    stopifnot(same(ordinary, zero))
  }
  for (kind in c("both", "nodesize")) {
    error <- tryCatch(run(kind, entries = entry[-1]), error = identity)
    stopifnot(inherits(error, "error"),
              grepl("original input data", conditionMessage(error)))
  }
}
check.tuning()
