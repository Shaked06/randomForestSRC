## Run from repository root: Rscript simulations/tests/package_cache.R
## Actual knitr cache hits across fresh R processes and same-version R/C
## package reinstalls. All installs, results, and caches are temporary.
local({
  repo <- normalizePath(".")
  documents <- sort(Sys.glob("simulations/*/stage1_main.Rmd"))
  stopifnot(length(documents) == 6L)
  headers <- vapply(documents, function(path) {
    header <- grep("^```\\{r loop,", readLines(path), value = TRUE)
    stopifnot(length(header) == 1L)
    header
  }, character(1))
  stopifnot(length(unique(headers)) == 1L)
  work <- tempfile("simulation-package-cache-")
  dir.create(work)
  on.exit(unlink(work, recursive = TRUE), add = TRUE)
  lib <- file.path(work, "library"); dir.create(lib)
  pkg <- file.path(work, "cacheFixture"); dir.create(pkg)
  dir.create(file.path(pkg, "R")); dir.create(file.path(pkg, "src"))
  writeLines(c("Package: cacheFixture", "Version: 0.0.1",
               "Title: Temporary Simulation Cache Fixture",
               "Description: A temporary package for a cache regression test.",
               "Author: Test", "Maintainer: Test <test@example.org>",
               "License: GPL-3"), file.path(pkg, "DESCRIPTION"))
  writeLines(c("useDynLib(cacheFixture)", "export(score)"), file.path(pkg, "NAMESPACE"))
  write.code <- function(r, native) {
    writeLines(sprintf('score <- function() %.3f + .Call("fixture_value", PACKAGE = "cacheFixture")', r),
               file.path(pkg, "R", "score.R"))
    writeLines(c('#include <Rinternals.h>',
                 sprintf("SEXP fixture_value(void) { return ScalarReal(%.3f); }", native)),
               file.path(pkg, "src", "fixture.c"))
  }
  run <- function(executable, args) {
    log <- suppressWarnings(system2(executable, shQuote(args), stdout = TRUE, stderr = TRUE))
    status <- attr(log, "status")
    if (!is.null(status) && status != 0L) stop(paste(log, collapse = "\n"))
    invisible(log)
  }
  install <- function() run(file.path(R.home("bin"), "R"),
                            c("CMD", "INSTALL", "--preclean", "--no-multiarch",
                              paste0("--library=", lib), pkg))
  project <- file.path(work, "scenario"); dir.create(project)
  common <- file.path(work, "_common"); dir.create(common)
  engine <- file.path(common, "sim_engine.R")
  stopifnot(file.copy(file.path(repo, "simulations/_common/sim_engine.R"), engine))
  saveRDS(list(test = TRUE), file.path(project, "setup.rds"))
  writeLines("## Unchanged DGP fixture.", file.path(project, "dgp.R"))
  writeLines(c(unname(headers[1]),
               "value <- cacheFixture::score()",
               'count <- if (file.exists("evaluations")) as.integer(readLines("evaluations")) else 0L',
               'writeLines(as.character(count + 1L), "evaluations")',
               "print(value)", "```"), file.path(project, "cache.Rmd"))
  child <- file.path(work, "knit.R")
  writeLines(c(
    "args <- commandArgs(TRUE)",
    ".libPaths(c(args[1], .libPaths()))",
    "setwd(args[2])",
    'source("../_common/sim_engine.R")',
    "N_REP <- 10L; M_TEST <- 200L",
    ## The NULL fallback lets the same regression reproduce stale caching
    ## on the previous engine, before the fingerprint function existed.
    'PACKAGE_PROVENANCE <- if (exists("simulation_package_provenance")) simulation_package_provenance("cacheFixture") else NULL',
    'knitr::opts_chunk$set(cache = TRUE, cache.path = "cache/")',
    "env <- new.env(parent = globalenv())",
    'knitr::knit("cache.Rmd", output = "output.md", envir = env, quiet = TRUE)',
    "saveRDS(list(direct = cacheFixture::score(), cached = env$value, provenance = PACKAGE_PROVENANCE), args[3])"
  ), child)
  knit <- function() {
    output <- file.path(work, "output.rds")
    run(file.path(R.home("bin"), "Rscript"), c("--vanilla", child, lib, project, output))
    readRDS(output)
  }
  count <- function() as.integer(readLines(file.path(project, "evaluations")))
  same <- function(x, y) isTRUE(all.equal(x, y, tolerance = 1e-12))
  write.code(.111, 0); install()
  first <- knit()
  stopifnot(same(first$direct, .111), same(first$cached, .111), count() == 1L)
  repeat.first <- knit()
  stopifnot(same(repeat.first$cached, .111), count() == 1L)

  ## Freeze metadata/native artifacts so only installed R code changes.
  installed <- file.path(lib, "cacheFixture")
  metadata <- readBin(file.path(installed, "DESCRIPTION"), "raw", n = 1e6)
  dll <- list.files(file.path(installed, "libs"), recursive = TRUE, full.names = TRUE)
  stopifnot(length(dll) == 1L)
  binary <- readBin(dll, "raw", n = file.info(dll)$size)
  write.code(.999, 0); install()
  writeBin(metadata, file.path(installed, "DESCRIPTION")); writeBin(binary, dll)
  r.change <- knit()
  stopifnot(same(r.change$direct, .999), same(r.change$cached, .999), count() == 2L)
  stopifnot(same(knit()$cached, .999), count() == 2L)

  ## Freeze R/metadata artifacts so only the compiled implementation changes.
  r.files <- list.files(file.path(installed, "R"), full.names = TRUE)
  r.bytes <- lapply(r.files, function(path) readBin(path, "raw", n = file.info(path)$size))
  write.code(.999, .25); install()
  writeBin(metadata, file.path(installed, "DESCRIPTION"))
  for (i in seq_along(r.files)) writeBin(r.bytes[[i]], r.files[i])
  native.change <- knit()
  stopifnot(same(native.change$direct, 1.249), same(native.change$cached, 1.249), count() == 3L)
  stopifnot(same(knit()$cached, 1.249), count() == 3L)
  provenance <- lapply(list(first, r.change, native.change), function(x) x$provenance$packages$cacheFixture)
  stopifnot(all(vapply(provenance, function(p) p$version == "0.0.1", logical(1))))
  hashes <- lapply(provenance, `[[`, "files")
  rdb <- grep("\\.rdb$", names(hashes[[1]]), value = TRUE)
  native <- grep("^libs/", names(hashes[[1]]), value = TRUE)
  stopifnot(length(rdb) > 0L, length(native) > 0L,
            identical(hashes[[1]]["DESCRIPTION"], hashes[[2]]["DESCRIPTION"]),
            identical(hashes[[2]]["DESCRIPTION"], hashes[[3]]["DESCRIPTION"]),
            !identical(hashes[[1]][rdb], hashes[[2]][rdb]),
            identical(hashes[[1]][native], hashes[[2]][native]),
            identical(hashes[[2]][rdb], hashes[[3]][rdb]),
            !identical(hashes[[2]][native], hashes[[3]][native]))
  ## Keep the original input/source invalidation as well as the new package key.
  writeLines("## Changed DGP fixture.", file.path(project, "dgp.R"))
  stopifnot(same(knit()$cached, 1.249), count() == 4L)
  saveRDS(list(test = FALSE), file.path(project, "setup.rds"))
  stopifnot(same(knit()$cached, 1.249), count() == 5L)
  cat("\n## Changed engine fixture.\n", file = engine, append = TRUE)
  stopifnot(same(knit()$cached, 1.249), count() == 6L)
  for (document in documents) {
    text <- paste(readLines(document), collapse = "\n")
    stopifnot(grepl("PACKAGE_PROVENANCE <- simulation_package_provenance()", text, fixed = TRUE),
              grepl("package_provenance = PACKAGE_PROVENANCE", text, fixed = TRUE))
  }
  cat("Package cache regression passed: unchanged installs hit cache; same-version R-only and native-only changes recompute; setup/DGP/engine changes still invalidate. All six scenarios use package provenance.\n")
})
