## =====================================================================
##  DGP — Cox proportional hazards with left truncation
## =====================================================================
##  Sourced by stage0_setup.Rmd, stage1_main.Rmd, and by every parallel
##  worker, so the model is defined exactly once.
##
##  Hazard      lambda(t|X) = lambda_0(t) exp(beta'X),  lambda_0(t) = 0.01 t
##  Cumulative  Lambda_0(t) = 0.005 t^2
##  Survival    S(t|X) = exp(-A0 t^2 exp(beta'X))
##
##  Functions are pure: no set.seed inside, so randomness stays under the
##  caller's control.
## =====================================================================

suppressPackageStartupMessages({
  library(MASS)      # mvrnorm
  library(Matrix)    # nearPD
})

A0   <- 0.005
P    <- 6L
BETA <- c(0.1, 0.1, 0.1, -0.1, -0.1, -0.1)

Lambda0 <- function(t) A0 * t^2


## ---------------------------------------------------------------------
##  Covariates
## ---------------------------------------------------------------------

build_sigma <- function(p = P, rho = 0.2, type = c("ar1", "cs")) {
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
##  S(T~|X) ~ U(0,1), so solving S(T~|X) = U gives a closed form and no
##  numerical solve is needed:
##      T~_i = sqrt( -log(U_i) / (A0 exp(beta'X_i)) )

sample_event_times <- function(X, beta = BETA) {
  lp <- as.vector(X %*% beta)
  U  <- runif(nrow(X))
  sqrt(-log(U) / (A0 * exp(lp)))
}

## Ground truth on a grid: m x M. Unchanged by truncation, which is a
## sampling mechanism rather than a change to the model.
true_survival_matrix <- function(Xtest, tgrid, beta = BETA) {
  lp <- as.vector(Xtest %*% beta)
  exp(-A0 * outer(exp(lp), tgrid^2))
}


## ---------------------------------------------------------------------
##  Calibration targets (all analytic)
## ---------------------------------------------------------------------
##  Each rate below integrates out the randomness that has not been drawn
##  yet, given the pilot's T~ values. That makes every calibration a
##  deterministic root-find rather than a noisy search.

## P(C < T~) for C ~ Exp(mean = theta), independent of T~. Decreasing in theta.
censoring_rate <- function(theta, Ttilde) mean(1 - exp(-Ttilde / theta))

## P(R > T~) for R ~ Unif(cL, cU), independent of T~. Increasing in cU.
trunc_rate <- function(cU, Ttilde, cL) {
  mean(pmin(pmax((cU - Ttilde) / (cU - cL), 0), 1))
}

## P(C < T~) among retained subjects when C = R + Exp(mean = theta).
## Decreasing in theta.
censoring_rate_lt <- function(theta, Ttilde_kept, R_kept) {
  mean(1 - exp(-(Ttilde_kept - R_kept) / theta))
}


## ---------------------------------------------------------------------
##  Samplers
## ---------------------------------------------------------------------

## Right-censored only, no truncation. Not consumed by the two-phase
## Stage 1; retained because it defines the untruncated reference DGP and
## calibrates theta.
make_survival_sample <- function(n, Sigma, theta, beta = BETA) {
  X      <- sample_X(n, Sigma)
  Ttilde <- sample_event_times(X, beta)
  C      <- rexp(n, rate = 1 / theta)
  colnames(X) <- paste0("x", seq_len(ncol(X)))
  data.frame(X, time = pmin(Ttilde, C), status = as.integer(Ttilde <= C))
}

## Left-truncated, right-censored.
##
## The retention filter uses the TRUE event time: a subject who died before
## its entry time was never enrolled. Truncation is a fact of nature,
## whereas censoring belongs to an observation process that only starts at
## entry.
##
## Censoring is therefore residual after entry, C = R + Exp(theta_lt). An
## origin-anchored C could land below R — a subject censored before it
## entered — which is incoherent, empties the delayed-entry risk set
## 1(R <= t <= T_obs), and is rejected by the fork's R-layer validation.
## This form guarantees entry <= time on every row.
make_ltrc_sample <- function(n, N_pool, Sigma, theta_lt, cU, cL, beta = BETA) {
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
             time   = pmin(Tk, Ck),
             status = as.integer(Tk <= Ck))
}


## ---------------------------------------------------------------------
##  The simulation object consumed by the shared engine
## ---------------------------------------------------------------------
##  draw_test samples covariates from the POPULATION distribution, not the
##  retained one. The retained sample is shifted toward low beta'X, but the
##  estimand is the conditional S(t|x), so evaluation belongs on population
##  covariates.

sim <- list(
  name = "01 — Cox PH DGP, left-truncated right-censored",

  draw_train = function(setup) {
    lt <- setup$ltrc
    make_ltrc_sample(lt$n_ltrc, lt$n_pool, setup$Sigma,
                     lt$theta_lt, lt$cU, lt$cL, setup$beta)
  },

  draw_test = function(setup, m) sample_X(m, setup$Sigma),

  truth = function(Xtest, tgrid, setup) {
    true_survival_matrix(Xtest, tgrid, setup$beta)
  }
)
