args <- commandArgs(TRUE)
run <- normalizePath(args[1]); .libPaths(c(args[2], .libPaths()))
n_rep <- as.integer(args[3]); n_cores <- as.integer(args[4])
log_connection <- file(file.path(run, paste0("refresh-", n_rep, ".log")), open = "at")
sink(log_connection, split = TRUE)
sink(log_connection, type = "message")
cat("INVOCATION START UTC", format(Sys.time(), tz = "UTC", usetz = TRUE), "target", n_rep, "workers", n_cores, "\n")
options(rf.cores = 1, mc.cores = 1)
source(file.path(run, "source/_common/sim_engine.R"))
source(file.path(run, "source/_common/sim_report.R"))
provenance <- simulation_package_provenance()
saveRDS(provenance, file.path(run, "package-provenance.rds"))
manifest <- jsonlite::read_json(file.path(run, "manifest.json"), simplifyVector = TRUE)
for (file in names(manifest$input_md5)) {
  relative <- sub("^simulations/", "", file)
  if (basename(relative) == "results_phases.rds") relative <- file.path(dirname(relative), "previous-results.rds")
  path <- file.path(run, "source", relative)
  stopifnot(unname(tools::md5sum(path)) == manifest$input_md5[[file]])
}
folders <- basename(dirname(Sys.glob(file.path(run, "source", "*", "setup.rds"))))
summary <- comparisons <- configurations <- list()
for (folder in sort(folders)) {
  cat("\nSCENARIO", folder, "started", format(Sys.time()), "\n")
  source(file.path(run, "source/_common/sim_engine.R"))
  source(file.path(run, "source", folder, "dgp.R"))
  setup <- readRDS(file.path(run, "source", folder, "setup.rds"))
  checkpoint <- file.path(run, "checkpoints", folder)
  dir.create(checkpoint, recursive = TRUE, showWarnings = FALSE)
  output <- file.path(run, "results", folder)
  dir.create(output, recursive = TRUE, showWarnings = FALSE)
  Sys.setenv(RFSRC_THESIS_CHECKPOINT_DIR = checkpoint,
             RFSRC_THESIS_SCENARIO_SOURCE = file.path(run, "source", folder),
             RFSRC_THESIS_FIT_LOG = file.path(checkpoint, "forest-fits.log"))
  source(file.path(run, "checkpoint_worker.R"))
  sources <- c(file.path(run, "source/_common/sim_engine.R"),
                file.path(run, "source", folder, "dgp.R"), file.path(run, "checkpoint_worker.R"))
  t0 <- Sys.time()
  result <- run_experiment(n_rep, setup, sim, sources, m_test = 200L, n_cores = n_cores,
                           verify_noop = TRUE, package_provenance = provenance)
  result$run_provenance <- list(source_commit = manifest$source_commit,
    setup_md5 = unname(tools::md5sum(file.path(run, "source", folder, "setup.rds"))),
    engine_md5 = unname(tools::md5sum(sources[1])),
    dgp_md5 = unname(tools::md5sum(sources[2])),
    n_cores = n_cores, verify_noop = TRUE,
    completed_utc = format(Sys.time(), tz = "UTC", usetz = TRUE))
  saveRDS(result, file.path(output, "results_phases.rds"))
  previous <- readRDS(file.path(run, "source", folder, "previous-results.rds"))
  for (field in c("overall", "cindex", "brier", "auc")) {
    current <- phase_table(result, field)
    old <- phase_table(previous, field)
    paired <- paired_shift(result, field = field)
    for (family in c("Cox", "RSF")) {
      row <- current[current$family == family & current$variant == "left-truncation", ]
      prior <- old[old$family == family & old$variant == "left-truncation", ]
      ci <- paired[grepl(family, paired$arm, fixed = TRUE), ]
      summary[[length(summary)+1L]] <- data.frame(scenario = folder, metric = field,
        family = family, n_rep = n_rep, phase1 = row$phase1, phase2 = row$phase2,
        improvement_percent = 100*(row$phase1-row$phase2)/row$phase1,
        paired_shift = ci$mean_shift, lower = ci$lower, upper = ci$upper,
        replications_improved = ci$n_moved_down)
      comparisons[[length(comparisons)+1L]] <- data.frame(scenario = folder, metric = field,
        family = family, previous_n_rep = previous$n_rep, current_n_rep = n_rep,
        previous_phase1 = prior$phase1, previous_phase2 = prior$phase2,
        current_phase1 = row$phase1, current_phase2 = row$phase2)
    }
  }
  configurations[[folder]] <- data.frame(scenario = folder,
    n_train = setup$ltrc$n_ltrc, n_pool = setup$ltrc$n_pool,
    ntree = setup$rsf_hp$ntree, mtry = setup$rsf_hp$mtry, nodesize = setup$rsf_hp$nodesize,
    n_rep = n_rep, m_test = 200L,
    zero_entry_cox_max_gap = max(result$noop$cox), zero_entry_rsf_max_gap = max(result$noop$rsf),
    wall_seconds_this_invocation = as.numeric(difftime(Sys.time(), t0, units = "secs")))
  write.csv(do.call(rbind, summary), file.path(run, "summary.csv"), row.names = FALSE)
  write.csv(do.call(rbind, comparisons), file.path(run, "comparison.csv"), row.names = FALSE)
  write.csv(do.call(rbind, configurations), file.path(run, "run_configuration.csv"), row.names = FALSE)
  cat("SCENARIO", folder, "completed", format(Sys.time()), "\n")
}
cat("ALL SCENARIOS COMPLETED\n")
