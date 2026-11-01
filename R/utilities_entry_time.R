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
