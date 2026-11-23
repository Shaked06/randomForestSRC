## Run from the repository root. Rebuild reports from the completed results.
repo <- normalizePath(".", mustWork = TRUE)
run_dir <- file.path(repo, "simulations/reruns/2026-11-06-1802cc3")
documents <- sort(Sys.glob(file.path(repo, "simulations/*/stage1_main.Rmd")))
stopifnot(length(documents) == 6L)
if (!rmarkdown::pandoc_available()) {
  bundled <- "/Applications/RStudio.app/Contents/Resources/app/quarto/bin/tools/aarch64"
  if (file.exists(file.path(bundled, "pandoc"))) {
    Sys.setenv(RSTUDIO_PANDOC = bundled)
    rmarkdown::find_pandoc(cache = FALSE)
  }
}
stopifnot(rmarkdown::pandoc_available())
if (identical(Sys.info()[["sysname"]], "Darwin")) options(bitmapType = "quartz")
data_files <- c(list.files(file.path(run_dir, "checkpoints"), pattern = "\\.rds$",
                           recursive = TRUE, full.names = TRUE),
                list.files(file.path(run_dir, "results"), pattern = "\\.rds$",
                           recursive = TRUE, full.names = TRUE),
                Sys.glob(file.path(repo, "simulations/*/results_phases.rds")))
before <- tools::md5sum(data_files)
for (document in documents) {
  scenario <- basename(dirname(document))
  cat(sprintf("Rendering %s from saved results\n", scenario))
  rmarkdown::render(document, params = list(recompute = FALSE),
                    envir = new.env(parent = globalenv()), quiet = TRUE)
}
cat("Rendering RESULTS.Rmd to HTML and Markdown from saved results\n")
rmarkdown::render(file.path(repo, "simulations/RESULTS.Rmd"), output_format = "all",
                    envir = new.env(parent = globalenv()), quiet = TRUE)
stopifnot(identical(before, tools::md5sum(data_files)))
cat(sprintf("Rendered %d scenario reports and the results summary; all %d result/checkpoint files unchanged.\n",
            length(documents), length(data_files)))
