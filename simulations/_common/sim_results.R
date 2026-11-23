## Load a completed experiment for reporting without fitting models or
## requiring the original package installation to remain on this machine.
read_completed_simulation <- function(run_dir, scenario) {
  run_dir <- normalizePath(run_dir, mustWork = TRUE)
  manifest <- jsonlite::read_json(file.path(run_dir, "manifest.json"),
                                  simplifyVector = TRUE)
  if (!identical(manifest$state, "complete")) {
    stop("Select a completed simulation run for reporting.")
  }
  result_file <- file.path(run_dir, "results", scenario, "results_phases.rds")
  setup_file <- file.path(run_dir, "source", scenario, "setup.rds")
  dgp_file <- file.path(run_dir, "source", scenario, "dgp.R")
  result <- readRDS(result_file)
  setup <- readRDS(setup_file)
  expected <- readRDS(file.path(run_dir, "package-provenance.rds"))
  stopifnot(result$n_rep == manifest$n_rep,
            result$m_test == manifest$test_subjects_per_replication,
            identical(result$package_provenance, expected),
            identical(result$run_provenance$source_commit, manifest$source_commit),
            identical(result$run_provenance$setup_md5, unname(tools::md5sum(setup_file))),
            identical(result$run_provenance$dgp_md5, unname(tools::md5sum(dgp_file))),
            nrow(result$noop) == result$n_rep,
            all(is.finite(as.matrix(result$noop))),
            all(result$noop$rsf == 0), all(result$noop$cox < 1e-8))
  for (field in c("overall", "cindex", "brier", "auc")) {
    scores <- as.matrix(result[[field]])
    stopifnot(!is.null(result[[field]]), nrow(scores) == result$n_rep,
              ncol(scores) == 8L, all(is.finite(scores)))
  }
  list(result = result, setup = setup, manifest = manifest,
       result_file = result_file)
}
