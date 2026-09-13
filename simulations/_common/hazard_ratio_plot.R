## =====================================================================
##  Hazard-ratio-over-time plot, all six simulations
## =====================================================================
##  For each DGP, picks one "low" and one "high" covariate profile
##  (active covariates moved apart, everything else held at its
##  distribution's centre), computes h(t|high)/h(t|low) on that
##  simulation's own calibrated time grid (from setup.rds), and indexes
##  the curve to 1 at its first grid point.
##
##  Reading the panels: a FLAT line at 1 means the hazard ratio between
##  these two subjects does not change over time -- proportional hazards
##  holds for this pair. A drifting line means it does not.
##
##  Run from the simulations/ directory:  Rscript _common/hazard_ratio_plot.R
## =====================================================================

SIM_DIR <- "."
OUT_PNG <- file.path(SIM_DIR, "_common", "hazard_ratios.png")

## dataviz reference palette (see simulations conversation / dataviz skill)
COL_LINE  <- "#2a78d6"   # categorical slot 1 (blue)
COL_REF   <- "#c3c2b7"   # baseline/axis (muted reference line at 1)
COL_INK   <- "#0b0b0b"   # primary ink (titles)
COL_INK2  <- "#52514e"   # secondary ink (axis labels)
COL_MUTED <- "#898781"   # muted ink (axis ticks)
COL_GRID  <- "#e1e0d9"   # hairline gridline
COL_SURF  <- "#fcfcfb"   # chart surface

## Build a 1 x P covariate row: `active` indices set to `active_val`,
## everything else at `background_val`.
make_profile <- function(P, active, val, background = 0) {
  x <- rep(background, P)
  x[active] <- val
  matrix(x, nrow = 1)
}

## ---------------------------------------------------------------------
## One hazard-ratio curve per simulation, each sourced into its own
## environment (several dgp.R files reuse names like mu_link / P).
## ---------------------------------------------------------------------

panels <- list()

## --- 01: Cox PH, h(t|X) = 0.01 t exp(beta'X) ---
e <- new.env(); source(file.path(SIM_DIR, "01_cox_random_forerst_no_lt/dgp.R"), local = e)
setup <- readRDS(file.path(SIM_DIR, "01_cox_random_forerst_no_lt/setup.rds"))
tg <- setup$time_grid
lo <- make_profile(e$P, seq_len(e$P), -1.5)   # all 6 covariates are active
hi <- make_profile(e$P, seq_len(e$P), +1.5)
h  <- function(X) 0.01 * tg * exp(as.vector(X %*% e$BETA))
panels[["01: Cox PH\n(no violation)"]] <- h(hi) / h(lo)

## --- 02: Gompertz-shape, h(t|X,Z) = 0.1 exp(a(X) t + theta0'Z) ---
e <- new.env(); source(file.path(SIM_DIR, "02_flexihaz_gompertz_lt/dgp.R"), local = e)
setup <- readRDS(file.path(SIM_DIR, "02_flexihaz_gompertz_lt/setup.rds"))
tg <- setup$time_grid
Z0 <- matrix(0, 1, 2)                          # Z held fixed; vary X (drives a(X))
Xlo <- matrix(-0.7, 1, 3); Xhi <- matrix(0.7, 1, 3)
h <- function(X, Zmat) e$BASE_RATE * exp(e$a_shape(X) * tg + as.vector(Zmat %*% e$THETA0))
panels[["02: Gompertz-shape\n(genuine PH violation)"]] <- h(Xhi, Z0) / h(Xlo, Z0)

## --- 03: RIST Sc.1, h(t|X) = exp(-beta'X) (constant in t) ---
e <- new.env(); source(file.path(SIM_DIR, "03_rist_scenario1_lt/dgp.R"), local = e)
setup <- readRDS(file.path(SIM_DIR, "03_rist_scenario1_lt/setup.rds"))
tg <- setup$time_grid
lo <- make_profile(e$P, e$ACTIVE, -1.5); hi <- make_profile(e$P, e$ACTIVE, +1.5)
h <- function(X) rep(exp(-as.vector(X %*% e$BETA)), length(tg))
panels[["03: RIST Sc.1\n(exactly Cox)"]] <- h(hi) / h(lo)

## --- 04: RIST Sc.2, h(t|X) = 1/mu(X) (constant in t) ---
e <- new.env(); source(file.path(SIM_DIR, "04_rist_scenario2_lt/dgp.R"), local = e)
setup <- readRDS(file.path(SIM_DIR, "04_rist_scenario2_lt/setup.rds"))
tg <- setup$time_grid
lo <- make_profile(e$P, e$ACTIVE, 0.15, background = 0.5)
hi <- make_profile(e$P, e$ACTIVE, 0.85, background = 0.5)
h <- function(X) rep(1 / e$mu_link(X), length(tg))
panels[["04: RIST Sc.2\n(nonlinear link, PH \"holds\" per-pair)"]] <- h(hi) / h(lo)

## --- 05: RIST Sc.3, Gamma hazard f(t)/S(t), shape = mu(X) ---
## mu(X) = 0.5 + 0.3*|sum(active)| -- a symmetric -1.5/+1.5 profile pair
## gives the SAME |sum| on both sides (abs() cancels the sign), which
## would make the ratio trivially flat for a reason that has nothing to
## do with the DGP. Compare |sum|=0 (baseline) against |sum| pushed high
## instead.
e <- new.env(); source(file.path(SIM_DIR, "05_rist_scenario3_lt/dgp.R"), local = e)
setup <- readRDS(file.path(SIM_DIR, "05_rist_scenario3_lt/setup.rds"))
tg <- setup$time_grid
lo <- make_profile(e$P, e$ACTIVE, 0); hi <- make_profile(e$P, e$ACTIVE, +1.5)
h <- function(X) {
  mu <- e$mu_shape(X)
  dgamma(tg, shape = mu, scale = e$GAMMA_SCALE) /
    pgamma(tg, shape = mu, scale = e$GAMMA_SCALE, lower.tail = FALSE)
}
panels[["05: RIST Sc.3\n(Gamma, genuine PH violation)"]] <- h(hi) / h(lo)

## --- 06: RIST Sc.4, Lognormal hazard f(t)/S(t), meanlog = mu(X) ---
## Same abs()-cancellation trap as 05: mu(X) sums two |.| terms, so a
## symmetric -1.5/+1.5 profile pair would land on the same mu by
## accident. Compare |sum|=0 (baseline) against |sum| pushed high.
e <- new.env(); source(file.path(SIM_DIR, "06_rist_scenario4_lt/dgp.R"), local = e)
setup <- readRDS(file.path(SIM_DIR, "06_rist_scenario4_lt/setup.rds"))
tg <- setup$time_grid
lo <- make_profile(e$P, c(e$ACTIVE_1, e$ACTIVE_2), 0)
hi <- make_profile(e$P, c(e$ACTIVE_1, e$ACTIVE_2), +1.5)
h <- function(X) {
  mu <- e$mu_link(X)
  dlnorm(tg, meanlog = mu, sdlog = e$SDLOG) /
    plnorm(tg, meanlog = mu, sdlog = e$SDLOG, lower.tail = FALSE)
}
panels[["06: RIST Sc.4\n(Lognormal, genuine PH violation)"]] <- h(hi) / h(lo)

## ---------------------------------------------------------------------
## Plot: 2x3 small multiples, each curve indexed to 1 at its first point.
## ---------------------------------------------------------------------

png(OUT_PNG, width = 1500, height = 950, res = 150, bg = COL_SURF)
op <- par(mfrow = c(2, 3), mar = c(4, 4.5, 3, 1.5), bg = COL_SURF,
          col.axis = COL_MUTED, col.lab = COL_INK2, fg = COL_MUTED)
on.exit(par(op), add = TRUE)

# reuse each panel's own tg (recomputed by name lookup, since tg was
# overwritten in the loop above) -- store alongside the ratio instead
tgrids <- list(
  "01: Cox PH\n(no violation)" = readRDS(file.path(SIM_DIR, "01_cox_random_forerst_no_lt/setup.rds"))$time_grid,
  "02: Gompertz-shape\n(genuine PH violation)" = readRDS(file.path(SIM_DIR, "02_flexihaz_gompertz_lt/setup.rds"))$time_grid,
  "03: RIST Sc.1\n(exactly Cox)" = readRDS(file.path(SIM_DIR, "03_rist_scenario1_lt/setup.rds"))$time_grid,
  "04: RIST Sc.2\n(nonlinear link, PH \"holds\" per-pair)" = readRDS(file.path(SIM_DIR, "04_rist_scenario2_lt/setup.rds"))$time_grid,
  "05: RIST Sc.3\n(Gamma, genuine PH violation)" = readRDS(file.path(SIM_DIR, "05_rist_scenario3_lt/setup.rds"))$time_grid,
  "06: RIST Sc.4\n(Lognormal, genuine PH violation)" = readRDS(file.path(SIM_DIR, "06_rist_scenario4_lt/setup.rds"))$time_grid
)

for (nm in names(panels)) {
  t   <- tgrids[[nm]]
  r   <- panels[[nm]]
  ridx <- r / r[1]                              # index to 1 at t[1]

  plot(t, ridx, log = "y", type = "n",
       xlab = "t", ylab = "hazard ratio (indexed to 1)",
       main = "", axes = FALSE)
  title(main = nm, col.main = COL_INK, font.main = 1, cex.main = 0.95)
  grid(col = COL_GRID, lty = 1)
  abline(h = 1, col = COL_REF, lty = 2, lwd = 1.5)
  lines(t, ridx, col = COL_LINE, lwd = 2)
  axis(1, col = COL_MUTED); axis(2, col = COL_MUTED)
  box(col = COL_GRID)
}

dev.off()
cat(sprintf("saved %s\n", OUT_PNG))
