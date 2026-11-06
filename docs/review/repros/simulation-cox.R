## Run from repository root. R_LIBS may select an isolated source installation.
## This characterizes a bug; no repository data or simulation results are changed.
suppressPackageStartupMessages(library(randomForestSRC))
options(rf.cores = 1)
source("simulations/_common/sim_engine.R")
d <- data.frame(time = c(1, 2, 3, 4, 6, 8, 9, 12),
                status = c(1, 1, 0, 1, 1, 0, 1, 0),
                x = c(-2, 1, 0, -1, 2, 0, 1, -1))
f <- coxph(Surv(time, status) ~ x, d)
tg <- c(0, 1, 2, 3, 4, 6, 8, 9, 12)
bh <- basehaz(f, centered = FALSE)
got <- drop(cox_survival_matrix(f, matrix(0, 1, dimnames = list(NULL, "x")), tg))
expected <- exp(-c(0, bh$hazard)[findInterval(tg, bh$time) + 1L])
stopifnot(any(diff(got) > 0), all(diff(expected) <= 0),
          max(abs(got - expected)) > .1)
cat("BUG Cox grid: returned survival at t=9,12 =", tail(got, 2),
    "; correct values =", tail(expected, 2), "\n")

## Optional: quantify impact on saved runs, without fitting any forests.
## This requires local ignored setup.rds and results_phases.rds files.
if ("--saved" %in% commandArgs(trailingOnly = TRUE)) {
  for (path in Sys.glob("simulations/*/setup.rds")) {
    saved.path <- file.path(dirname(path), "results_phases.rds")
    if (!file.exists(saved.path)) next
    env <- new.env(parent = globalenv())
    sys.source(file.path(dirname(path), "dgp.R"), env)
    s <- readRDS(path)
    saved <- readRDS(saved.path)
    for (phase in 1:2) {
      vals <- t(vapply(seq_len(saved$n_rep), function(r) {
        set.seed(s$meta$master_seed + 1000L + r)
        train <- env$sim$draw_train(s)
        xn <- setdiff(names(train), c("entry", "time", "status"))
        xt <- env$sim$draw_test(s, saved$m_test)
        if (is.null(colnames(xt))) colnames(xt) <- xn
        train$.entry <- if (phase == 1) rep(0, nrow(train)) else train$entry
        f <- coxph(as.formula(paste("Surv(.entry,time,status)~",
                                    paste(xn, collapse = "+"))), train)
        old <- cox_survival_matrix(f, xt, s$time_grid)
        bh <- basehaz(f, centered = FALSE)
        idx <- findInterval(s$time_grid, bh$time)
        H <- c(0, bh$hazard)[idx + 1L]
        lp <- as.vector(xt[, names(coef(f)), drop = FALSE] %*% coef(f))
        corrected <- exp(-outer(exp(lp), H))
        truth <- env$sim$truth(xt, s$time_grid, s)
        c(old = mean(abs(old - truth)), corrected = mean(abs(corrected - truth)),
          affected = as.numeric(any(idx == 0)))
      }, numeric(3)))
      saved.error <- saved$overall[[paste0("p", phase, "_cox_lt")]]
      stopifnot(max(abs(vals[, "old"] - saved.error)) < 1e-10)
      cat(sprintf("%s phase %d: saved/reproduced MAD %.8f; corrected %.8f; affected %d/%d\n",
                  basename(dirname(path)), phase, mean(vals[, "old"]),
                  mean(vals[, "corrected"]), sum(vals[, "affected"]), saved$n_rep))
    }
  }
}
