.thesis_original_replication <- run_replication
if (!exists(".thesis_fit_trace_installed", inherits = FALSE)) {
  invisible(trace("rfsrc", where = as.environment("package:randomForestSRC"), print = FALSE,
    tracer = quote(cat(format(Sys.time()), "pid", Sys.getpid(), "forest started",
      "ntree", ntree, "entry", if (is.null(entry.time)) "NULL" else if (any(entry.time > 0)) "positive" else "zero",
      "\n", file = Sys.getenv("RFSRC_THESIS_FIT_LOG"), append = TRUE)),
    exit = quote(cat(format(Sys.time()), "pid", Sys.getpid(), "forest finished\n",
      file = Sys.getenv("RFSRC_THESIS_FIT_LOG"), append = TRUE))))
  .thesis_fit_trace_installed <- TRUE
}
run_replication <- function(rep_id, setup, sim, scorer = scorer_mad_vs_truth,
                            m_test = 200L, verify_noop = FALSE) {
  directory <- Sys.getenv("RFSRC_THESIS_CHECKPOINT_DIR")
  input <- Sys.getenv("RFSRC_THESIS_SCENARIO_SOURCE")
  key <- tools::md5sum(file.path(input, c("setup.rds", "dgp.R")))
  provenance <- simulation_package_provenance()
  path <- file.path(directory, sprintf("rep-%03d.rds", rep_id))
  validate <- function(checkpoint) {
    stopifnot(identical(checkpoint$rep_id, as.integer(rep_id)),
              identical(checkpoint$verify_noop, verify_noop),
              identical(checkpoint$input_md5, key),
              identical(checkpoint$package_provenance, provenance))
    result <- checkpoint$result
    if (verify_noop) stopifnot(result$noop["rsf"] == 0, result$noop["cox"] < 1e-8)
    for (field in c("overall", "cindex", "brier", "auc")) {
      stopifnot(length(result[[field]]) == 8L, all(is.finite(result[[field]])))
    }
    result
  }
  if (file.exists(path)) return(validate(readRDS(path)))
  cat(format(Sys.time()), "replication", rep_id, "started\n",
      file = file.path(directory, "progress.log"), append = TRUE)
  t0 <- proc.time()
  result <- .thesis_original_replication(rep_id, setup, sim, scorer, m_test, verify_noop)
  timing <- proc.time() - t0
  checkpoint <- list(rep_id = as.integer(rep_id), result = result,
    elapsed_seconds = unname(timing["elapsed"]),
    cpu_seconds = unname(sum(timing[c("user.self", "sys.self")])), verify_noop = verify_noop,
    package_provenance = provenance, input_md5 = key)
  validate(checkpoint)
  temporary <- paste0(path, ".tmp-", Sys.getpid())
  saveRDS(checkpoint, temporary)
  stopifnot(file.rename(temporary, path))
  cat(format(Sys.time()), "replication", rep_id, "completed in", checkpoint$elapsed_seconds, "seconds\n",
      file = file.path(directory, "progress.log"), append = TRUE)
  result
}
