## Run from repository root with the current fork installed:
## Rscript simulations/tests/package_provenance.R
## Exercises worker library selection, rejection of mismatched provenance,
## and saved result metadata using a small real experiment. No saved runs change.
local({
  source("simulations/_common/sim_engine.R", local = TRUE)
  source("simulations/01_cox_random_forerst_lt/dgp.R", local = TRUE)
  stopifnot("entry.time" %in% names(formals(randomForestSRC::rfsrc)))
  engine <- normalizePath("simulations/_common/sim_engine.R")
  sources <- c(engine, normalizePath("simulations/01_cox_random_forerst_lt/dgp.R"))
  set.seed(93)
  rng <- .Random.seed
  provenance <- simulation_package_provenance()
  stopifnot(identical(rng, .Random.seed),
            identical(provenance, simulation_package_provenance()),
            identical(names(provenance$packages), c("randomForestSRC", "survival")),
            all(vapply(provenance$packages, function(p) {
              any(grepl("\\.rdb$", names(p$files))) && any(grepl("^libs/", names(p$files)))
            }, logical(1))))

  ## The packages can have been loaded from a library no longer on the
  ## parent's search path. Workers must still receive that installation.
  libraries <- .libPaths()
  on.exit(.libPaths(libraries), add = TRUE)
  .libPaths(setdiff(libraries, dirname(provenance$packages$randomForestSRC$path)))
  ## Remove inherited library overrides too, so the worker cannot find an
  ## isolated installation accidentally through its startup environment.
  library.env <- Sys.getenv(c("R_LIBS", "R_LIBS_USER", "R_LIBS_SITE"), unset = NA_character_)
  restore.library.env <- function() {
    Sys.unsetenv(names(library.env)[is.na(library.env)])
    do.call(Sys.setenv, as.list(library.env[!is.na(library.env)]))
  }
  on.exit(restore.library.env(), add = TRUE)
  Sys.setenv(R_LIBS = "", R_LIBS_USER = "", R_LIBS_SITE = "")
  cl <- spawn_cluster(engine, 1L, provenance)
  observed <- tryCatch(parallel::clusterCall(cl, function() simulation_package_provenance()),
                        finally = parallel::stopCluster(cl))
  stopifnot(identical(observed[[1]], provenance))
  .libPaths(libraries)
  restore.library.env()

  wrong <- provenance
  wrong$packages$randomForestSRC$files[1] <- "intentionally-mismatched-test-fingerprint"
  sockets <- function() sum(showConnections(all = FALSE)[, "class"] == "sockconn")
  before <- sockets()
  result <- tryCatch(spawn_cluster(engine, 1L, wrong), error = identity)
  if (!inherits(result, "error")) {
    parallel::stopCluster(result)
    stop("A worker accepted mismatched package provenance")
  }
  stopifnot(grepl("Worker package implementation differs", conditionMessage(result), fixed = TRUE),
            sockets() == before)

  setup <- list(meta = list(master_seed = 42L), Sigma = diag(6),
                 beta = c(.1, .1, .1, -.1, -.1, -.1),
                 ltrc = list(n_ltrc = 120L, n_pool = 3000L,
                             theta_lt = 5, cL = 0, cU = 2),
                 rsf_hp = list(ntree = 12L, mtry = 2L, nodesize = 5L),
                 time_grid = c(1, 2, 4, 6, 8, 10, 12))
  ## A changed key must be rejected before launching the experiment.
  result <- tryCatch(run_experiment(2L, setup, sim, sources, n_cores = 1L,
                                    package_provenance = wrong), error = identity)
  stopifnot(inherits(result, "error"),
            grepl("Installed packages changed", conditionMessage(result), fixed = TRUE))

  actual <- run_experiment(2L, setup, sim, sources, m_test = 15L, n_cores = 1L,
                           verify_noop = TRUE, package_provenance = provenance)
  reference <- lapply(1:2, run_replication, setup = setup, sim = sim,
                       m_test = 15L, verify_noop = TRUE)
  for (field in c("overall", "noop", "cindex", "brier", "auc")) {
    expected <- as.data.frame(do.call(rbind, lapply(reference, `[[`, field)))
    stopifnot(isTRUE(all.equal(actual[[field]], expected, tolerance = 1e-12)))
  }
  stopifnot(identical(actual$package_provenance, provenance),
            all(actual$noop$rsf == 0), all(actual$noop$cox < 1e-10))
  saved <- tempfile("simulation-provenance-", fileext = ".rds")
  on.exit(unlink(saved), add = TRUE)
  saveRDS(actual, saved)
  stopifnot(identical(readRDS(saved)$package_provenance, provenance))
  cat("Package provenance regression passed: worker installation matches, mismatches stop and close sockets, serial/worker experiment scores agree, saved provenance survives serialization.\n")
})
