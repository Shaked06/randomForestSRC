args <- commandArgs(TRUE)
run <- normalizePath(args[1]); library <- args[2]
repeat {
  log <- file.path(run, "refresh-100.log")
  text <- if (file.exists(log)) readLines(log, warn = FALSE) else character()
  if (any(grepl("ALL SCENARIOS COMPLETED", text, fixed = TRUE))) break
  if (any(grepl("Execution halted", text, fixed = TRUE))) stop("Simulation stopped; checkpoints preserved. See refresh-100.log.")
  Sys.sleep(30)
}
status <- system2(file.path(R.home("bin"), "Rscript"),
  shQuote(c(file.path(run, "render_reports.R"), run, library)),
  stdout = file.path(run, "render.log"), stderr = file.path(run, "render.log"))
if (status != 0L) stop("Report rendering failed; completed simulation results are preserved. See render.log.")
cat("All 100-replication scenario results and reports are complete.\n")
