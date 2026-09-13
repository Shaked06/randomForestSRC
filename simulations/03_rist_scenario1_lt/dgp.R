## =====================================================================
##  DGP — RIST paper, Scenario 1 (p. 334), with left truncation added
## =====================================================================
##  Source: Zhu and Kosorok (2012), "Recursively Imputed Survival Trees,"
##  JASA 107(497), 331-340, Section 4.1, Scenario 1
##  (../../papers/Recursively Imputed Survival Trees.pdf).
##  Scenario 1 is itself "adapted from Section 4 of" Ishwaran, Kogalur,
##  Gorodeski, Minn, and Lauer (2010), "High-Dimensional Variable
##  Selection for Survival Data," JASA 105(489), 205-217, Section 4
##  (../../papers/High-Dimensional_Variable_Selection_for_Survival_D.pdf).
##
##  As printed, RIST's Scenario 1 gives the survival-time mean as the raw
##  linear sum mu = b0 * sum(x11..x20), which is mean-zero (X is standard
##  normal) and so negative about half the time -- not a valid Exponential
##  mean. The source it cites resolves this: Ishwaran et al.'s own Section 4
##  uses mu = exp(sum(beta_k x_k)), i.e. the sum is a linear predictor that
##  gets exponentiated, not the mean itself. We follow the source paper's
##  exp() here; RIST's restatement is read as having dropped it.
##
##  What is taken from the papers, unchanged:
##    p = 25, X ~ N(0, V), V_ij = rho^|i-j|, rho = 0.9        (RIST Sc. 1)
##    mu(X) = exp(b0 * sum(x11..x20)), b0 = 0.1                (source paper's
##                                                               exp() form)
##    T ~ Exponential(mean = mu(X))                            (RIST Sc. 1)
##    C ~ Exponential(mean = theta), theta a single shared      (RIST Sc. 1)
##      value calibrated from the sample (not per-subject)
##    tau = 4 (administrative censoring)                        (RIST Sc. 1)
##    target censoring rate ~30%                                (RIST Sc. 1)
##
##  What we add, not in either paper (neither has left truncation):
##    entry time R ~ Unif(cL, cU), retention filtered on R <= Ttilde,
##    censoring residual after entry (C = R + Exp(theta_lt)) so that
##    entry <= time on every row -- the same mechanism used in
##    simulations 01 and 02. cU, theta, and theta_lt are all calibrated
##    by root-find against a target rate rather than taken from a formula,
##    following this project's established pattern (see stage0_setup.Rmd):
##    the paper's ~30% is treated as the design target for both the
##    untruncated reference theta and the retained-population theta_lt.
##
##  Functions are pure: no set.seed inside, so randomness stays under the
##  caller's control.
## =====================================================================

suppressPackageStartupMessages({
  library(MASS)      # mvrnorm
  library(Matrix)    # nearPD
})

P    <- 25L
B0   <- 0.1
ACTIVE <- 11:20                 # RIST Sc. 1: sum(x11..x20)
BETA <- { b <- numeric(P); b[ACTIVE] <- B0; b }
TAU  <- 4


## ---------------------------------------------------------------------
##  Covariates
## ---------------------------------------------------------------------
##  V_ij = rho^|i-j|, rho = 0.9 -- an AR(1) correlation structure, same
##  form as simulation 01's build_sigma() but at RIST Sc. 1's own rho.

build_sigma <- function(p = P, rho = 0.9, type = c("ar1", "cs")) {
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


## ---------------------------------------------------------------------
##  Event times by inverse transform
## ---------------------------------------------------------------------
##  T ~ Exponential(mean = mu(X)), mu(X) = exp(beta'X). S(Ttilde|X) ~
##  U(0,1), so solving S(Ttilde|X) = U gives Ttilde = -mu(X) * log(U),
##  the same closed form used in simulation 01.

sample_event_times <- function(X, beta = BETA) {
  mu <- exp(as.vector(X %*% beta))
  U  <- runif(nrow(X))
  -mu * log(U)
}

## Ground truth on a grid: m x M. S(t|X) = exp(-t / mu(X)) = exp(-t *
## exp(-beta'X)). Unchanged by truncation, which is a sampling mechanism
## rather than a change to the model.
true_survival_matrix <- function(Xtest, tgrid, beta = BETA) {
  lp <- as.vector(Xtest %*% beta)
  exp(-outer(exp(-lp), tgrid))
}


## ---------------------------------------------------------------------
##  Calibration targets (all analytic, admin censoring at tau folded in)
## ---------------------------------------------------------------------
##  Each rate integrates out the randomness that has not been drawn yet,
##  given the pilot's Ttilde values, making calibration a deterministic
##  root-find. A subject with Ttilde > tau is censored regardless of C,
##  which pmin(Ttilde, tau) captures without a branch.

## P(observed censored) for C ~ Exp(mean = theta), admin cutoff at tau.
## Decreasing in theta.
censoring_rate <- function(theta, Ttilde, tau = TAU) {
  mean(1 - exp(-pmin(Ttilde, tau) / theta))
}

## P(R > Ttilde) for R ~ Unif(cL, cU), independent of Ttilde. Increasing
## in cU.
trunc_rate <- function(cU, Ttilde, cL) {
  mean(pmin(pmax((cU - Ttilde) / (cU - cL), 0), 1))
}

## P(censored | retained) when C = R + Exp(mean = theta), admin cutoff at
## tau. Decreasing in theta.
censoring_rate_lt <- function(theta, Ttilde_kept, R_kept, tau = TAU) {
  mean(1 - exp(-(pmin(Ttilde_kept, tau) - R_kept) / theta))
}


## ---------------------------------------------------------------------
##  Samplers
## ---------------------------------------------------------------------

## Right-censored only, no truncation. Not consumed by the two-phase
## Stage 1; retained because it defines the untruncated reference DGP and
## calibrates theta. Unlike RIST Sc. 1's own theta = 0.5 * mean(mu), theta
## here is root-found against TARGET_CENSOR in stage0_setup.Rmd -- see the
## note at the top of this file.
make_survival_sample <- function(n, Sigma, theta, tau = TAU, beta = BETA) {
  X      <- sample_X(n, Sigma)
  Ttilde <- sample_event_times(X, beta)
  C      <- rexp(n, rate = 1 / theta)
  colnames(X) <- paste0("x", seq_len(ncol(X)))
  data.frame(X, time = pmin(Ttilde, C, tau),
             status = as.integer(Ttilde <= pmin(C, tau)))
}

## Left-truncated, right-censored. Same mechanism as simulations 01/02:
## the retention filter uses the true event time (truncation is a fact of
## nature), and censoring is residual after entry, C = R + Exp(theta_lt),
## which guarantees entry <= time on every row.
make_ltrc_sample <- function(n, N_pool, Sigma, theta_lt, cU, cL,
                             tau = TAU, beta = BETA) {
  X      <- sample_X(N_pool, Sigma)
  Ttilde <- sample_event_times(X, beta)
  R      <- runif(N_pool, cL, cU)

  keep <- which(R <= Ttilde)
  if (length(keep) < n) {
    stop(sprintf("pool too small: %d retained, need %d", length(keep), n))
  }
  idx <- sample(keep, n)

  Xk <- X[idx, , drop = FALSE]
  Tk <- Ttilde[idx]
  Rk <- R[idx]
  Ck <- Rk + rexp(n, rate = 1 / theta_lt)

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
  name = "03 - RIST Scenario 1 DGP (p=25, rho=0.9), left-truncated right-censored",

  draw_train = function(setup) {
    lt <- setup$ltrc
    make_ltrc_sample(lt$n_ltrc, lt$n_pool, setup$Sigma,
                     lt$theta_lt, lt$cU, lt$cL, setup$tau, setup$beta)
  },

  draw_test = function(setup, m) sample_X(m, setup$Sigma),

  truth = function(Xtest, tgrid, setup) {
    true_survival_matrix(Xtest, tgrid, setup$beta)
  }
)
