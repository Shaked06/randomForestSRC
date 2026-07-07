## Quick end-to-end demo: does the LTRC-aware RSF actually run and produce
## sensible, different results from the standard (truncation-ignoring) RSF?
## Not a rigorous validation (that's stage0/stage1) -- just a "does it work" check.

suppressPackageStartupMessages({
  library(randomForestSRC)
})

set.seed(42)

## ---- Small synthetic left-truncated survival dataset ----
## Same DGP shape as stage0_setup.Rmd (Lambda0(t) = A0*t^2), but simplified,
## plus a left-truncation mechanism: subjects enter the study at a random
## point AFTER time 0 (e.g. delayed enrollment), and we only observe those
## who are still alive/uncensored at their own entry time (the standard
## LTRC "only L_i <= T~_i are observed" condition).
n  <- 400
p  <- 4
A0 <- 0.01
beta <- c(0.4, -0.4, 0.3, -0.3)

X <- matrix(rnorm(n * p), n, p)
colnames(X) <- paste0("x", 1:p)
lp <- as.vector(X %*% beta)

U <- runif(n)
Ttrue <- sqrt(-log(U) / (A0 * exp(lp)))         # true event time, inverse-transform
C     <- rexp(n, rate = 1/15)                    # independent censoring
Tobs  <- pmin(Ttrue, C)
Delta <- as.integer(Ttrue <= C)

## Left-truncation: entry time uniform on (0, 0.6 * Tobs) -- guarantees L_i < Tobs_i.
## Every simulated subject is "observed" by construction (L_i <= Tobs_i), matching
## the LTRC observed-data assumption from TASK.md.
entry <- runif(n, 0, 0.6 * Tobs)

dat <- data.frame(X, time = Tobs, status = Delta, entry = entry)
cat(sprintf("n=%d, events=%d (%.0f%%), median follow-up=%.2f, median entry=%.2f\n",
            n, sum(Delta), 100*mean(Delta), median(Tobs), median(entry)))

## ---- Fit 1: standard RSF, truncation ignored ----
set.seed(1)
fit_std <- rfsrc(Surv(time, status) ~ x1 + x2 + x3 + x4, data = dat, ntree = 200)

## ---- Fit 2: our new LTRC-aware RSF ----
set.seed(1)
fit_ltrc <- rfsrc(Surv(time, status) ~ x1 + x2 + x3 + x4, data = dat, ntree = 200,
                  entry.time = dat$entry)

cat("\n--- Standard RSF (ignores entry) ---\n")
print(fit_std)

cat("\n--- LTRC RSF (entry.time supplied) ---\n")
print(fit_ltrc)

## ---- Sanity: do the two fits actually differ? ----
cat("\nOOB error, standard :", fit_std$err.rate[length(fit_std$err.rate)], "\n")
cat("OOB error, LTRC      :", fit_ltrc$err.rate[length(fit_ltrc$err.rate)], "\n")
cat("chf identical:", isTRUE(all.equal(fit_std$chf, fit_ltrc$chf)), "(expect FALSE)\n")
cat("Any non-finite chf values (LTRC):", any(!is.finite(fit_ltrc$chf)), "(expect FALSE)\n")

## ---- Regression check: entry.time = 0 must reproduce standard RSF exactly ----
set.seed(1)
fit_zero <- rfsrc(Surv(time, status) ~ x1 + x2 + x3 + x4, data = dat, ntree = 200,
                   entry.time = rep(0, n))
cat("entry.time=0 reproduces standard RSF exactly:",
    isTRUE(all.equal(fit_std$chf, fit_zero$chf)), "(expect TRUE)\n")

## ---- Predict on a small held-out set ----
newX <- as.data.frame(matrix(rnorm(5 * p), 5, p))
colnames(newX) <- paste0("x", 1:p)
pr <- predict(fit_ltrc, newdata = newX)
cat("\nPredicted survival matrix on 5 new subjects: dim =", dim(pr$survival), "\n")
cat("All predicted survival probabilities in [0,1]:",
    all(pr$survival >= 0 & pr$survival <= 1), "\n")

cat("\nDemo complete: LTRC RSF runs end-to-end, differs from standard RSF,\n")
cat("collapses to standard RSF when entry.time=0, and predicts cleanly on new data.\n")
