## From the repository root:
## Rscript docs/thesis/generate_results_tables.R [completed-run-directory]
## Reads saved results only; never fits models.
args <- commandArgs(trailingOnly = TRUE)
run <- if (length(args)) args[1] else "simulations/reruns/2026-11-06-1802cc3"
manifest <- jsonlite::read_json(file.path(run, "manifest.json"), simplifyVector = TRUE)
stopifnot(manifest$state == "complete", manifest$n_rep == 100L)
config <- read.csv(file.path(run, "run_configuration.csv"))
stopifnot(nrow(config) == 6L, all(config$n_rep == 100L))
provenance <- readRDS(file.path(run, "package-provenance.rds"))
results <- lapply(config$scenario, function(folder) {
  r <- readRDS(file.path(run, "results", folder, "results_phases.rds"))
  stopifnot(r$n_rep == 100L, r$m_test == 200L,
            all(r$noop$rsf == 0), all(r$noop$cox < 1e-8),
            identical(r$package_provenance, provenance))
  r
})
dest <- file.path("docs/thesis",
                  "Extending_randomForestSRC_to_SupportLeft_Truncated_Survival_Data_Thesis_draft_for_discussion",
                  "tables")
dir.create(dest, recursive = TRUE, showWarnings = FALSE)
line <- function(cells) paste0(paste(cells, collapse = " & "), " \\\\")
number <- function(x, digits) formatC(x, format = "f", digits = digits)
percent <- function(x) paste0(number(x, 2), "\\%")
statistics <- function(r, field, family) {
  e0 <- r[[field]][[paste0("p1_", family, "_lt")]]
  eL <- r[[field]][[paste0("p2_", family, "_lt")]]
  stopifnot(length(e0) == 100L, length(eL) == 100L, all(is.finite(c(e0, eL))))
  d <- e0 - eL
  c(blind = mean(e0), aware = mean(eL), gain = mean(d),
    mcse = sd(d)/sqrt(length(d)), reduction = 100*mean(d)/mean(e0))
}
write.score.table <- function(field, file, title, label, digits = 4L, reduction = FALSE) {
  rows <- character()
  for (i in seq_along(results)) for (family in c("cox", "rsf")) {
    s <- statistics(results[[i]], field, family)
    cells <- c(i, if (family == "cox") "Cox" else "RSF",
               number(s[c("blind", "aware", "gain")], digits),
               number(s["mcse"], if (field %in% c("auc", "cindex")) 6 else 5))
    if (reduction) cells <- c(cells, percent(s["reduction"]))
    rows <- c(rows, line(cells))
  }
  header <- c("Scenario", "Model", "Entry-blind", "Entry-aware", "Gain", "MCSE")
  if (reduction) header <- c(header, "Reduction")
  writeLines(c(
    "% Generated from the completed 100-replication run; do not edit by hand.",
    "\\begin{table}[htbp]", "\\centering\\small", "\\setlength{\\tabcolsep}{4pt}",
    paste0("\\begin{tabular}{cl", strrep("r", length(header) - 2), "}"),
    "\\toprule", line(header), "\\midrule", rows, "\\bottomrule", "\\end{tabular}",
    paste0("\\caption{", title, " Gain is mean entry-blind minus entry-aware error; positive values favor entry-aware fitting. MCSE is the Monte Carlo standard error of the paired mean gain. ",
           if (reduction) "Reduction is the gain divided by mean entry-blind error. " else "",
           "There are \\SimReplications{} replications per scenario.}"),
    paste0("\\label{", label, "}"), "\\end{table}"), file.path(dest, file))
}
write.score.table("overall", "mad-results.tex", "Mean absolute survival-curve error (MAD).", "tab:mad", reduction = TRUE)
write.score.table("cindex", "concordance-results.tex", "Entry-aware concordance error, $1-C$.", "tab:concordance", digits = 6)
write.score.table("brier", "brier-results.tex", "Integrated entered-cohort Brier score (unnormalized CRPS). The integration horizon differs between scenarios.", "tab:brier", digits = 5)
write.score.table("auc", "auc-results.tex", "Error $1-\\overline{\\mathrm{AUC}}$ for the descriptive time average of entered-cohort AUC.", "tab:auc", digits = 6)

rows <- vapply(seq_along(results), function(i) {
  reductions <- unlist(lapply(c("overall", "cindex", "brier", "auc"), function(field) {
    vapply(c("cox", "rsf"), function(family) statistics(results[[i]], field, family)["reduction"], numeric(1))
  }))
  line(c(i, vapply(reductions, percent, character(1))))
}, character(1))
writeLines(c("% Generated from the completed run; percentage changes are on error scales.",
  "\\begin{table}[htbp]", "\\centering\\small", "\\setlength{\\tabcolsep}{3pt}",
  "\\begin{tabular}{crrrrrrrr}", "\\toprule",
  line(c("", "\\multicolumn{2}{c}{MAD}", "\\multicolumn{2}{c}{Concordance error}",
         "\\multicolumn{2}{c}{CRPS}", "\\multicolumn{2}{c}{AUC error}")),
  "\\cmidrule(lr){2-3}\\cmidrule(lr){4-5}\\cmidrule(lr){6-7}\\cmidrule(lr){8-9}",
  line(c("Scenario", rep(c("Cox", "RSF"), 4))), "\\midrule", rows,
  "\\bottomrule", "\\end{tabular}",
  "\\caption{Percentage reduction in mean error after supplying entry times, $100(\\overline E_0-\\overline E_L)/\\overline E_0$. Positive values favor entry-aware fitting; negative values indicate a higher error. Concordance and AUC columns use $1-C$ and $1-\\overline{\\mathrm{AUC}}$, respectively. These are relative error changes, not percentage-point changes in C or AUC.}",
  "\\label{tab:metric-reductions}", "\\end{table}"), file.path(dest, "metric-reductions.tex"))

writeLines(c("% Generated run metadata.",
  "\\newcommand{\\SimReplications}{100}", "\\newcommand{\\SimTrainN}{5000}",
  "\\newcommand{\\SimTrees}{500}", "\\newcommand{\\SimTestN}{200}",
  paste0("\\newcommand{\\RunCommit}{\\texttt{", substr(manifest$source_commit, 1, 7), "}}")),
  file.path(dest, "run-info.tex"))
rows <- vapply(seq_len(nrow(config)), function(i) line(c(i, config$mtry[i], config$nodesize[i])), character(1))
writeLines(c("% Generated from the preserved run configuration.",
  "\\begin{table}[htbp]", "\\centering", "\\begin{tabular}{crr}",
  "\\toprule", line(c("Scenario", "mtry", "nodesize")), "\\midrule", rows,
  "\\bottomrule", "\\end{tabular}",
  "\\caption{Preserved forest hyperparameters. Every scenario uses \\SimTrees{} trees and \\SimTrainN{} retained training subjects per replication. No retuning was performed for this refresh.}",
  "\\label{tab:hyperparameters}", "\\end{table}"), file.path(dest, "hyperparameters.tex"))
writeLines(c("% Compatibility file: the current numerical table is in mad-results.tex.",
             "\\input{tables/mad-results}"), file.path(dest, "preliminary-mad.tex"))
cat("Generated all result tables from", run, "at commit", substr(manifest$source_commit, 1, 7), "\n")
