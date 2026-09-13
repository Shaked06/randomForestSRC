## =====================================================================
##  DGP — RIST paper, Scenario 3 (p. 334), with left truncation added
## =====================================================================
##  Source: Zhu and Kosorok (2012), "Recursively Imputed Survival Trees,"
##  JASA 107(497), 331-340, Section 4.1, Scenario 3
##  (../../papers/Recursively Imputed Survival Trees.pdf).
##
##  As printed: "Let p = 25 and X = (X1,...,X25) be drawn from a
##  multivariate normal distribution with covariance matrix V, where
##  Vij = rho^|i-j| and rho is set to 0.75. Survival times are drawn
##  independently from a gamma distribution with shape parameter
##  mu = 0.5 + 0.3*|sum_{i=11}^{15} xi| and scale parameter 2. Censoring
##  times are drawn uniformly from (0, 1.5*tau), and the study length tau
##  is set to 10. The sample size is 300, and the censoring rate is
##  approximately 20%."
##
##  Only x11..x15 (5 of the 25 covariates) drive the Gamma shape; the rest
##  are noise. Survival times are Gamma, not Exponential (Scenarios 1/2) --
##  a genuinely different distributional family, not just a different
##  link/covariate structure.
##
##  A subtlety this scenario has that Scenarios 1/2 do not: the censoring
##  window's own bound (1.5*tau = 15) is WIDER than the administrative
##  cutoff tau = 10. So unlike simulation 04 (Scenario 2), where C could
##  never exceed the study horizon by construction, here a subject can be
##  right-censored two structurally different ways: C landing before
##  Ttilde while both are still under tau, or Ttilde itself exceeding tau
##  (administrative censoring, regardless of C). censoring_rate() and
##  censoring_rate_lt() below branch on this explicitly rather than
##  folding tau into a single expression the way simulations 02/03 do for
##  their Exponential censoring -- that folding is only exact when the
##  censoring distribution is memoryless (Exponential); for a bounded
##  Uniform window it is not, so the two cases are kept separate here.
##
##  What is taken from the paper, unchanged:
##    p = 25, X ~ N(0, V), V_ij = rho^|i-j|, rho = 0.75
##    mu(X) = 0.5 + 0.3*|sum(x11..x15)|
##    T ~ Gamma(shape = mu(X), scale = 2)
##    C ~ Uniform(0, U), U a single shared upper bound (not per-subject)
##    tau = 10 (administrative censoring)
##    target censoring rate ~20%
##
##  What we add, not in the paper (Scenario 3 has no left truncation):
##    entry time R ~ Unif(cL, cU), retention filtered on R <= Ttilde,
##    same mechanism as simulations 01-04. Residual censoring after entry
##    keeps the paper's Uniform shape, as in simulation 04:
##    C = R + Unif(0, w_lt). cU, U (reference), and w_lt are all
##    calibrated by root-find against a target rate, following this
##    project's established pattern; the paper's ~20% is treated as the
##    design target for both.
##
##  Functions are pure: no set.seed inside, so randomness stays under the
##  caller's control.
## =====================================================================

suppressPackageStartupMessages({
  library(MASS)      # mvrnorm
  library(Matrix)    # nearPD
})

P            <- 25L
ACTIVE       <- 11:15          # RIST Sc. 3: sum(x11..x15)
MU_INTERCEPT <- 0.5
MU_SLOPE     <- 0.3
GAMMA_SCALE  <- 2
TAU          <- 10             # administrative censoring cutoff


## ---------------------------------------------------------------------
##  Covariates
## ---------------------------------------------------------------------
##  V_ij = rho^|i-j|, rho = 0.75 -- same AR(1) form as simulation 03, at
##  Scenario 3's own rho.

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

## Gamma shape parameter: 0.5 + 0.3*|sum(x11..x15)|, always >= 0.5.
mu_shape <- function(X) {
  s <- rowSums(X[, ACTIVE, drop = FALSE])
  MU_INTERCEPT + MU_SLOPE * abs(s)
}


## ---------------------------------------------------------------------
##  Event times by inverse transform
## ---------------------------------------------------------------------
##  T ~ Gamma(shape = mu(X), scale = 2). The Gamma CDF has no closed-form
##  inverse, so unlike simulations 01-04's algebraic inverse-transform,
##  this uses R's numerically inverted qgamma() -- still inverse-transform
##  sampling (Ttilde = F^{-1}(U)), just via a numerical F^{-1} rather than
##  an algebraic one.

sample_event_times <- function(X) {
  mu <- mu_shape(X)
  U  <- runif(nrow(X))
  qgamma(U, shape = mu, scale = GAMMA_SCALE)
}

## Ground truth on a grid: m x M. S(t|X) = 1 - F_Gamma(t; shape=mu(X),
## scale=2). Unlike simulations 01-04, the shape parameter differs by row,
## so pgamma cannot be called once on an outer-product grid; vapply loops
## over test subjects (m is small -- the Stage 1 test-set size). Unchanged
## by truncation, which is a sampling mechanism rather than a change to
## the model. Xtest must carry columns x1..xP (only x11..x15 are read).
true_survival_matrix <- function(Xtest, tgrid) {
  mu <- mu_shape(Xtest)
  t(vapply(mu, function(m) pgamma(tgrid, shape = m, scale = GAMMA_SCALE,
                                   lower.tail = FALSE),
            numeric(length(tgrid))))
}


## ---------------------------------------------------------------------
##  Calibration targets (all analytic, admin censoring at tau handled
##  exactly -- see the module note above on why this branches)
## ---------------------------------------------------------------------

## P(observed censored) for C ~ Unif(0, U), admin cutoff at tau.
## Ttilde > tau: censored regardless of C (admin cutoff binds first).
## Ttilde <= tau: censored iff C < Ttilde, P(C < t) = min(t, U) / U.
## Decreasing in U on the first branch (wider window, less likely C lands
## below Ttilde); the second branch does not depend on U.
censoring_rate <- function(U, Ttilde, tau = TAU) {
  mean(ifelse(Ttilde > tau, 1, pmin(Ttilde, U) / U))
}

## P(R > Ttilde) for R ~ Unif(cL, cU), independent of Ttilde. Increasing
## in cU. Unaffected by tau -- entry timing and the administrative cutoff
## are separate mechanisms.
trunc_rate <- function(cU, Ttilde, cL) {
  mean(pmin(pmax((cU - Ttilde) / (cU - cL), 0), 1))
}

## P(censored | retained) when C = R + Unif(0, w), admin cutoff at tau.
## Same branching as censoring_rate(): Ttilde > tau censors regardless of
## the residual draw; otherwise censored iff the residual wait undercuts
## the remaining lifetime, W < Ttilde - R.
censoring_rate_lt <- function(w, Ttilde_kept, R_kept, tau = TAU) {
  d <- Ttilde_kept - R_kept
  mean(ifelse(Ttilde_kept > tau, 1, pmin(d, w) / w))
}


## ---------------------------------------------------------------------
##  Samplers
## ---------------------------------------------------------------------

## Right-censored only, no truncation. Not consumed by the two-phase
## Stage 1; retained because it defines the untruncated reference DGP and
## calibrates the censoring upper bound U. Unlike RIST Sc. 3's own fixed
## U = 1.5*tau = 15, U here is root-found against TARGET_CENSOR in
## stage0_setup.Rmd -- see the note at the top of this file.
make_survival_sample <- function(n, Sigma, cens_upper, tau = TAU) {
  X      <- sample_X(n, Sigma)
  Ttilde <- sample_event_times(X)
  C      <- runif(n, 0, cens_upper)
  colnames(X) <- paste0("x", seq_len(ncol(X)))
  data.frame(X, time = pmin(Ttilde, C, tau),
             status = as.integer(Ttilde <= pmin(C, tau)))
}

## Left-truncated, right-censored. Same mechanism as simulations 01-04:
## the retention filter uses the true event time, and censoring is
## residual after entry -- C = R + Unif(0, w_lt), keeping the paper's own
## Uniform shape -- which guarantees entry <= time on every row.
make_ltrc_sample <- function(n, N_pool, Sigma, w_lt, cU, cL, tau = TAU) {
  X      <- sample_X(N_pool, Sigma)
  Ttilde <- sample_event_times(X)
  R      <- runif(N_pool, cL, cU)

  keep <- which(R <= Ttilde)
  if (length(keep) < n) {
    stop(sprintf("pool too small: %d retained, need %d", length(keep), n))
  }
  idx <- sample(keep, n)

  Xk <- X[idx, , drop = FALSE]
  Tk <- Ttilde[idx]
  Rk <- R[idx]
  Wk <- runif(n, 0, w_lt)
  Ck <- Rk + Wk

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
  name = "05 - RIST Scenario 3 DGP (p=25, Gamma survival times), left-truncated right-censored",

  draw_train = function(setup) {
    lt <- setup$ltrc
    make_ltrc_sample(lt$n_ltrc, lt$n_pool, setup$Sigma,
                     lt$w_lt, lt$cU, lt$cL, setup$tau)
  },

  draw_test = function(setup, m) sample_X(m, setup$Sigma),

  truth = function(Xtest, tgrid, setup) true_survival_matrix(Xtest, tgrid)
)
