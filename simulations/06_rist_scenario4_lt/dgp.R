## =====================================================================
##  DGP — RIST paper, Scenario 4 (p. 334), with left truncation added
## =====================================================================
##  Source: Zhu and Kosorok (2012), "Recursively Imputed Survival Trees,"
##  JASA 107(497), 331-340, Section 4.1, Scenario 4
##  (../../papers/Recursively Imputed Survival Trees.pdf).
##
##  As printed: "We generate a conditionally independent censoring setting
##  where p = 25 and X = (X1,...,X25) are drawn from a multivariate normal
##  distribution with covariance matrix V, where Vij = rho^|i-j| and rho
##  is set to 0.75. Survival times are drawn independently from a
##  lognormal distribution with mean set to
##  mu = 0.1*|sum_{i=1}^{5} xi| + 0.1*|sum_{i=21}^{25} xi|. Censoring
##  times follow the same distribution with parameter mu + 0.5. The study
##  length tau is set to 4."
##
##  "Conditionally independent censoring" is the scenario's own name for
##  its design: T and C are drawn independently of each other, but both
##  depend on the same X through mu(X) -- independent given X, not
##  unconditionally independent (unlike Scenarios 1-3, where C's
##  distribution did not depend on X at all).
##
##  A gap the paper leaves open: no spread (sdlog) is given for the
##  lognormal, only its location (meanlog = mu(X) for T, mu(X)+0.5 for C).
##  We take sdlog = 1, R's own rlnorm()/qlnorm() default and the most
##  natural reading when a paper writes "lognormal with mean mu" and
##  stops there.
##
##  What is taken from the paper, unchanged:
##    p = 25, X ~ N(0, V), V_ij = rho^|i-j|, rho = 0.75
##    mu(X) = 0.1*|sum(x1..x5)| + 0.1*|sum(x21..x25)|
##    T ~ Lognormal(meanlog = mu(X), sdlog = 1)
##    C ~ Lognormal(meanlog = mu(X) + delta, sdlog = 1), independent of T
##      given X -- delta is 0.5 in the paper
##    tau = 4 (administrative censoring)
##    target censoring rate ~32%
##
##  What we add, not in the paper (Scenario 4 has no left truncation):
##    entry time R ~ Unif(cL, cU), retention filtered on R <= Ttilde, same
##    mechanism as simulations 01-05. Residual censoring after entry keeps
##    the paper's Lognormal shape and its X-dependence: C = R + W, W ~
##    Lognormal(meanlog = mu(X) + delta_lt, sdlog = 1) -- a residual wait
##    duration after entry rather than an absolute time, mirroring how
##    simulations 01/03 turn an absolute Exponential censoring time into
##    a residual one and simulations 02/04/05 do the same for Uniform.
##    cU, delta (reference), and delta_lt are calibrated by root-find
##    against a target rate, following this project's established
##    pattern, rather than the paper's fixed delta = 0.5. RIST's ~32% is
##    the design target for both.
##
##  Because C's parameter depends on X (unlike Scenarios 1-3), the
##  calibration functions below take mu(X) as an explicit argument
##  alongside Ttilde, rather than depending on Ttilde alone.
##
##  A structural interaction worth knowing about: truncation enriches the
##  retained set for subjects with longer Ttilde (same effect noted in
##  simulations 03/05), and longer Ttilde means more of them run past
##  tau=4 before anything else happens -- so admin censoring alone is a
##  larger share of retained subjects (~37% in a 20k-pilot check) than of
##  the whole pool (~24%). That floor sits above RIST's ~32% target, so
##  the retained-population censoring rate cannot be calibrated down to
##  32% no matter how the extra censoring window is set. stage0_setup.Rmd
##  detects this and falls back to disabling the extra window (admin
##  censoring only) rather than reporting a target it cannot reach.
##
##  Functions are pure: no set.seed inside, so randomness stays under the
##  caller's control.
## =====================================================================

suppressPackageStartupMessages({
  library(MASS)      # mvrnorm
  library(Matrix)    # nearPD
})

P        <- 25L
ACTIVE_1 <- 1:5          # RIST Sc. 4: sum(x1..x5)
ACTIVE_2 <- 21:25        # RIST Sc. 4: sum(x21..x25)
MU_COEF  <- 0.1
SDLOG    <- 1            # not given by the paper -- see the note above
TAU      <- 4             # administrative censoring cutoff


## ---------------------------------------------------------------------
##  Covariates
## ---------------------------------------------------------------------
##  V_ij = rho^|i-j|, rho = 0.75 -- same AR(1) form and rho as
##  simulation 05 (Scenario 3).

build_sigma <- function(p = P, rho = 0.75, type = c("ar1", "cs")) {
  type <- match.arg(type)
  S <- if (type == "cs") {
    M <- matrix(rho, p, p); diag(M) <- 1; M
  } else {
    rho ^ abs(outer(seq_len(p), seq_len(p), "-"))
  }
  if (min(eigen(S, symmetric = TRUE, only.values = TRUE)$values) <= 1e-8) {
    S <- as.matrix(Matrix::nearPD(S, corr = TRUE)$mat)
  }
  S
}

sample_X <- function(n, Sigma) {
  MASS::mvrnorm(n = n, mu = rep(0, ncol(Sigma)), Sigma = Sigma)
}

## Lognormal location parameter: two disjoint groups of 5 covariates each
## (x1..x5 and x21..x25), always >= 0.
mu_link <- function(X) {
  s1 <- rowSums(X[, ACTIVE_1, drop = FALSE])
  s2 <- rowSums(X[, ACTIVE_2, drop = FALSE])
  MU_COEF * abs(s1) + MU_COEF * abs(s2)
}


## ---------------------------------------------------------------------
##  Event times by inverse transform
## ---------------------------------------------------------------------
##  T ~ Lognormal(meanlog = mu(X), sdlog = 1). Like simulation 05's Gamma,
##  the Lognormal CDF has no closed-form inverse, so this uses R's
##  numerically inverted qlnorm() -- still inverse-transform sampling.

sample_event_times <- function(X) {
  mu <- mu_link(X)
  U  <- runif(nrow(X))
  qlnorm(U, meanlog = mu, sdlog = SDLOG)
}

## Ground truth on a grid: m x M. S(t|X) = 1 - F_Lognormal(t; mu(X), 1).
## The location parameter differs by row, so plnorm cannot be called once
## on an outer-product grid; vapply loops over test subjects (m is small
## -- the Stage 1 test-set size), same pattern as simulation 05. Unchanged
## by truncation. Xtest must carry columns x1..xP (only x1..x5, x21..x25
## are read).
true_survival_matrix <- function(Xtest, tgrid) {
  mu <- mu_link(Xtest)
  t(vapply(mu, function(m) plnorm(tgrid, meanlog = m, sdlog = SDLOG,
                                   lower.tail = FALSE),
            numeric(length(tgrid))))
}


## ---------------------------------------------------------------------
##  Calibration targets (all analytic, admin censoring at tau handled
##  exactly, as in simulation 05)
## ---------------------------------------------------------------------
##  Unlike simulations 01-05, C's own parameter depends on X (mu(X)), so
##  these take mu as an explicit argument. The pilot's *realized* Ttilde
##  values are still treated as given, exactly as in earlier simulations
##  -- only C's own randomness is integrated out analytically, via
##  plnorm's closed-form CDF.

## P(observed censored) for C ~ Lognormal(meanlog = mu(X) + delta,
## sdlog = 1), admin cutoff at tau. Ttilde > tau: censored regardless of
## C. Ttilde <= tau: censored iff C < Ttilde, P(C < t) = plnorm(t; mu+delta,
## 1). Decreasing in delta (raising C's location shifts its whole
## distribution later, so it is less likely to land below Ttilde).
censoring_rate <- function(delta, Ttilde, mu, tau = TAU) {
  mean(ifelse(Ttilde > tau, 1,
              plnorm(Ttilde, meanlog = mu + delta, sdlog = SDLOG)))
}

## P(R > Ttilde) for R ~ Unif(cL, cU), independent of Ttilde. Increasing
## in cU. Unaffected by tau or by mu(X).
trunc_rate <- function(cU, Ttilde, cL) {
  mean(pmin(pmax((cU - Ttilde) / (cU - cL), 0), 1))
}

## P(censored | retained) when C = R + W, W ~ Lognormal(meanlog =
## mu(X) + delta, sdlog = 1), admin cutoff at tau. Same branching as
## censoring_rate(): Ttilde > tau censors regardless of W; otherwise
## censored iff the residual wait undercuts the remaining lifetime,
## W < Ttilde - R.
censoring_rate_lt <- function(delta, Ttilde_kept, R_kept, mu_kept, tau = TAU) {
  d <- Ttilde_kept - R_kept
  mean(ifelse(Ttilde_kept > tau, 1,
              plnorm(d, meanlog = mu_kept + delta, sdlog = SDLOG)))
}


## ---------------------------------------------------------------------
##  Samplers
## ---------------------------------------------------------------------

## Right-censored only, no truncation. Not consumed by the two-phase
## Stage 1; retained because it defines the untruncated reference DGP and
## calibrates the location shift delta. Unlike RIST Sc. 4's own fixed
## delta = 0.5, delta here is root-found against TARGET_CENSOR in
## stage0_setup.Rmd -- see the note at the top of this file.
make_survival_sample <- function(n, Sigma, delta, tau = TAU) {
  X      <- sample_X(n, Sigma)
  mu     <- mu_link(X)
  Ttilde <- qlnorm(runif(n), meanlog = mu, sdlog = SDLOG)
  C      <- qlnorm(runif(n), meanlog = mu + delta, sdlog = SDLOG)
  colnames(X) <- paste0("x", seq_len(ncol(X)))
  data.frame(X, time = pmin(Ttilde, C, tau),
             status = as.integer(Ttilde <= pmin(C, tau)))
}

## Left-truncated, right-censored. Same mechanism as simulations 01-05:
## the retention filter uses the true event time, and censoring is
## residual after entry -- C = R + W, W ~ Lognormal(meanlog =
## mu(X) + delta_lt, sdlog = 1), keeping the paper's own Lognormal shape
## and X-dependence -- which guarantees entry <= time on every row.
make_ltrc_sample <- function(n, N_pool, Sigma, delta_lt, cU, cL, tau = TAU) {
  X      <- sample_X(N_pool, Sigma)
  mu     <- mu_link(X)
  Ttilde <- qlnorm(runif(N_pool), meanlog = mu, sdlog = SDLOG)
  R      <- runif(N_pool, cL, cU)

  keep <- which(R <= Ttilde)
  if (length(keep) < n) {
    stop(sprintf("pool too small: %d retained, need %d", length(keep), n))
  }
  idx <- sample(keep, n)

  Xk  <- X[idx, , drop = FALSE]
  muk <- mu[idx]
  Tk  <- Ttilde[idx]
  Rk  <- R[idx]
  Wk  <- qlnorm(runif(n), meanlog = muk + delta_lt, sdlog = SDLOG)
  Ck  <- Rk + Wk

  colnames(Xk) <- paste0("x", seq_len(ncol(Xk)))
  data.frame(Xk, entry = Rk,
             time   = pmin(Tk, Ck, tau),
             status = as.integer(Tk <= pmin(Ck, tau)))
}


## ---------------------------------------------------------------------
##  The simulation object consumed by the shared engine
## ---------------------------------------------------------------------
##  draw_test samples covariates from the POPULATION distribution, not the
##  retained one -- the estimand is the conditional S(t|x).

sim <- list(
  name = "06 - RIST Scenario 4 DGP (p=25, Lognormal, conditionally independent censoring), left-truncated right-censored",

  draw_train = function(setup) {
    lt <- setup$ltrc
    make_ltrc_sample(lt$n_ltrc, lt$n_pool, setup$Sigma,
                     lt$delta_lt, lt$cU, lt$cL, setup$tau)
  },

  draw_test = function(setup, m) sample_X(m, setup$Sigma),

  truth = function(Xtest, tgrid, setup) true_survival_matrix(Xtest, tgrid)
)
