## Recover training entry times from a forest. The fallback supports grow
## objects saved before entry.time was also stored inside their forest.
get.forest.entry.time <- function(object, fallback = NULL) {
  entry.time <- object$entry.time
  if (is.null(entry.time)) entry.time <- fallback
  if (is.null(entry.time)) return(rep(0, object$n))
  if (!is.numeric(entry.time) || length(entry.time) != object$n ||
      anyNA(entry.time) || any(!is.finite(entry.time)) || any(entry.time < 0)) {
    stop("Invalid entry.time stored in forest: expected one finite, non-negative value per training observation.")
  }
  if (grepl("surv", object$family) && length(object$yvar.names) == 2) {
    validate.entry.time.followup(entry.time, object$yvar[, 1])
  }
  as.double(entry.time)
}

validate.entry.time.followup <- function(entry.time, time) {
  ## Ordinary response imputation does not enforce a subject-specific lower
  ## bound. Guard both growing and restoration before native imputation.
  if (any(is.na(time) & entry.time > 0)) {
    stop("Missing follow-up times cannot be imputed for subjects with positive entry.time.")
  }
  if (any(entry.time > time, na.rm = TRUE)) {
    stop("Invalid entry.time vector specified: entry.time must not exceed the observed follow-up time for any subject.")
  }
  invisible(NULL)
}

## The native new-data error has no evaluation-entry argument. Tuners keep
## entry times alongside the holdout rows and restrict concordance pairs
## here, reusing prediction's training-derived Uno weights when present.
get.tuning.holdout.error <- function(prediction, entry.time = NULL) {
  if (is.null(entry.time) || !any(entry.time > 0) ||
      prediction$family != "surv" || is.null(prediction$err.rate)) {
    return(mean(get.mv.error(prediction, TRUE), na.rm = TRUE))
  }
  yvar <- prediction$yvar
  if (is.null(yvar) || nrow(yvar) != length(entry.time)) {
    stop("Held-out entry times must align with held-out outcomes.")
  }
  args <- list(time = yvar[, 1], censoring = yvar[, 2],
               predicted = prediction$predicted, entry = entry.time)
  if (!is.null(prediction$uno.weights)) args$weight <- prediction$uno.weights
  do.call(get.cindex, args)
}
