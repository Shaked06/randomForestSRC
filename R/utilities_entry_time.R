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
  as.double(entry.time)
}
