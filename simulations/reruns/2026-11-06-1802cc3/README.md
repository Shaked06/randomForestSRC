# Simulation refresh after the bug fixes

Package/source commit: `1802cc3`. The package uses an isolated installation.

All six experiments completed **100 replications per experiment (600 total)**. The expanded run reused the first ten checkpoints per experiment and fitted the remainder with two workers. Each replication uses 5,000 training subjects, 500 trees per forest, 200 test covariates, and independent zero-entry checks.

Inputs and previous results are preserved locally under `source/`. Checkpoints are saved atomically under `checkpoints/<scenario>/rep-NNN.rds`. The original simulation results and pending research edits remain untouched.

`refresh-100.log` records the expanded run. New checkpoints contain separate CPU and elapsed seconds; runtime estimates exclude the already-cached replications. The engine's raw ETA includes cached work and can be misleading during startup.

`finish_reports.R` waits for all six 100-replication scenarios, then renders the overview and scenario reports without additional model fitting. `REPORT.md`, `summary.csv`, `comparison.csv`, and `run_configuration.csv` describe the completed output. The manifest records the completed run and its input/package fingerprints.

No Git commit or push is performed by these scripts.

The six original simulations/<scenario>/stage1_main.html reports were refreshed from the completed 100-replication reports on 7 November 2026. Their previous HTML copies are preserved under source/<scenario>/previous-stage1_main.html. Original saved RDS files and pending research edits remain untouched.


The six original `stage1_main.Rmd` documents now render directly, with their
original R Markdown HTML settings. Their default `recompute: false` mode
reads and validates these completed results. Run `Rscript
simulations/reruns/2026-11-06-1802cc3/render_rmd.R` from the repository root
to rebuild all six reports and the HTML/Markdown results summary. The script verifies that all 600 checkpoints and
both historical/current combined results remain unchanged. The prior RMDs
and Markdown-derived HTMLs are archived under
`source/before-direct-rmd-render-2026-11-08/`.

`simulations/RESULTS.Rmd` defaults to HTML when knitted; it also supports
`github_document` for the Markdown summary. Both outputs use the completed
100-replication results. Previous summary files are archived under
`source/before-direct-summary-rmd-render-2026-11-08/`.
