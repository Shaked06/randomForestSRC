args <- commandArgs(TRUE)
run <- normalizePath(args[1]); .libPaths(c(args[2], .libPaths()))
summary <- read.csv(file.path(run, "summary.csv"), check.names = FALSE)
config <- read.csv(file.path(run, "run_configuration.csv"), check.names = FALSE)
stopifnot(nrow(config) == 6L, nrow(summary) == 48L,
          all(config$zero_entry_rsf_max_gap == 0), all(config$zero_entry_cox_max_gap < 1e-8))
manifest <- jsonlite::read_json(file.path(run, "manifest.json"), simplifyVector = TRUE)
report_matches_target <- all(config$n_rep == manifest$n_rep)
options(bitmapType = "quartz")
strip.yaml <- function(lines) {
  if (length(lines) && lines[1] == "---") {
    end <- which(lines[-1] == "---")[1] + 1L
    lines <- lines[-seq_len(end)]
  }
  lines
}
render <- function(lines, directory, output, figures) {
  dir.create(directory, recursive = TRUE, showWarnings = FALSE)
  previous <- setwd(directory); on.exit(setwd(previous), add = TRUE)
  options <- knitr::opts_chunk$get()
  on.exit(knitr::opts_chunk$set(options), add = TRUE)
  knitr::opts_chunk$set(cache = FALSE, echo = FALSE, warning = FALSE, message = FALSE,
                       fig.path = figures, dev = "png")
  knitr::knit(text = paste(lines, collapse = "\n"), output = output,
              envir = new.env(parent = globalenv()), quiet = TRUE)
}
results <- file.path(run, "results")
dir.create(file.path(results, "_common"), showWarnings = FALSE)
stopifnot(file.copy(file.path(run, "source/_common/sim_report.R"),
                   file.path(results, "_common/sim_report.R"), overwrite = TRUE))
lines <- strip.yaml(readLines(file.path(run, "source/RESULTS.Rmd")))
render(lines, results, "RESULTS.md", "RESULTS_files/figure-gfm/")
body <- readLines(file.path(results, "RESULTS.md"))
body <- gsub("(RESULTS_files/", "(results/RESULTS_files/", body, fixed = TRUE)
body <- gsub("see `README.md`", "see the [simulation overview](../../README.md)", body, fixed = TRUE)
header <- c("# Refreshed simulation results", "",
  paste0("Package/source commit: `", substr(manifest$source_commit, 1, 7), "`."),
  paste0("Completed: ", format(Sys.time(), tz = "UTC", usetz = TRUE), "."), "",
  "The six scenarios use their preserved setup files and original seeds, with 5,000 training subjects, 500 trees per forest, and 200 population test covariates per replication. No hyperparameters were retuned. Both phases use the same data within each replication.", "",
  "Every replication independently checks ordinary versus zero-entry Cox/RSF fits. These checks pass. The package implementation is fingerprinted in each saved result, and workers verify the matching installation.", "",
  "This is a refresh after the bug fixes and upstream integration. Historical results lack package provenance. Changes from historical numbers can reflect corrected scoring, the Cox lookup fix, prediction/restoration fixes, and upstream censoring/RNG changes; the comparison does not isolate individual fixes.", "",
  "MAD evaluates unconditioned survival curves against source-population truth. The C-index, entered-cohort Brier/CRPS, and AUC use training outcomes (RSF OOB curves; Cox in-sample curves). The simulation C-index helper is unweighted; Brier/AUC use the pooled delayed-entry Kaplan-Meier censoring model. These metrics assess different targets. The pooled model assumes a common censoring hazard; scenarios with censoring depending on covariates/entry require care in interpreting these descriptive scores. AUC is our extension, not a prescribed estimator in the professor's outline.", "",
  "The original saved simulations and pending research edits remain untouched. Fresh result objects are under `results/<scenario>/results_phases.rds`; `summary.csv` contains paired phase shifts and descriptive 95% normal-approximation intervals, while `comparison.csv` records historical/current means. The six original stage1_main.html reports were refreshed from these 100-replication results on 7 November 2026; previous HTML copies are preserved under source/.", "",
  "## Run configuration", "", knitr::kable(config[, c("scenario", "n_train", "ntree", "mtry", "nodesize", "n_rep", "m_test")]), "",
  "## Results", "")
writeLines(c(header, body), file.path(run, "REPORT.md"))
for (scenario in config$scenario) {
  source <- file.path(run, "source", scenario)
  lines <- strip.yaml(readLines(file.path(source, "stage1_main.Rmd")))
  start <- which(startsWith(lines, "```{r loop,"))
  stopifnot(length(start) == 1L)
  end <- start + which(lines[(start+1L):length(lines)] == "```")[1]
  replacement <- c("```{r loop, cache=FALSE}",
    sprintf('res <- readRDS("%s")', file.path(run, "results", scenario, "results_phases.rds")),
    "N_REP <- res$n_rep; M_TEST <- res$m_test", "```")
  lines <- c(lines[seq_len(start-1L)], replacement, lines[(end+1L):length(lines)])
  for (name in c("sim_engine.R", "sim_report.R")) {
    lines <- gsub(paste0('source("../_common/', name, '")'),
                   sprintf('source("%s")', file.path(run, "source/_common", name)), lines, fixed = TRUE)
  }
  lines <- gsub('source("dgp.R")', sprintf('source("%s")', file.path(source, "dgp.R")), lines, fixed = TRUE)
  lines <- gsub('readRDS("setup.rds")', sprintf('readRDS("%s")', file.path(source, "setup.rds")), lines, fixed = TRUE)
  lines <- gsub("cache   = TRUE,", "cache   = FALSE,", lines, fixed = TRUE)
  text <- paste(lines, collapse = "\n")
  text <- gsub("Scoring against the analytic truth rather than out-of-bag error avoids the\ndelayed-entry C-index and Brier score, which are not implemented in the fork.",
               "MAD checks source-population survival truth; the additional concordance, Brier, and AUC sections use the current entry-aware scoring helpers.", text, fixed = TRUE)
  text <- gsub("**Phase 1** should produce four equal numbers.",
               "**Phase 1** should reproduce equality within each ordinary/zero-entry pair.", text, fixed = TRUE)
  prefix <- c(paste0("# Refreshed results: ", scenario), "",
               "Rendered from the preserved Stage 1 document with its fitting loop replaced by loading the completed refreshed result. This report performs no new fits. Original setup/source snapshots are preserved locally.", "")
  render(c(prefix, text), file.path(run, "reports"), paste0(scenario, ".md"), paste0("figures/", scenario, "/"))
}
provenance <- readRDS(file.path(run, "package-provenance.rds"))
provenance$packages <- lapply(provenance$packages, function(package) {
  package$files <- as.list(package$files); package
})
manifest$package_provenance <- provenance
manifest$state <- if (report_matches_target) "complete" else "running"
manifest$report_n_rep <- unique(config$n_rep)
manifest$report_created_utc <- format(Sys.time(), tz = "UTC", usetz = TRUE)
if (report_matches_target) manifest$completed_utc <- manifest$report_created_utc
jsonlite::write_json(manifest, file.path(run, "manifest.json"), pretty = TRUE, auto_unbox = TRUE, digits = NA)
cat("Refreshed overview and all six scenario reports rendered as Markdown without Pandoc.\n")
