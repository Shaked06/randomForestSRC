## =====================================================================
##  DGP — Section 7.1 simulation design (Gompertz-shape hazard), with
##  left truncation added
## =====================================================================
##  Source: "Flexible Deep Neural Networks for Partially Linear Survival
##  Data", Section 7.1 (papers/Survival-Deep2512.10570v2.pdf). We take
##  their data-generating process, not their neural estimators — the
##  fitted arms here are Cox and RSF, as in simulation 01, so the
##  left-truncation question is measured on a structurally different DGP.
##
##  Nuisance covariates  X = (X1,X2,X3), primary covariates Z = (Z1,Z2),
##  all iid Unif[-1,1].
##  f(X)      = 0.2(X1+X2) + 0.5 X1 X2 + X3^2      (nonlinear, interaction)
##  a(X)      = 0.1 + f(X)^2                        (time-slope of hazard)
##  Hazard    h(t|X,Z) = 0.1 exp{ a(X) t + theta0'Z },  t in [0, tau]
##  theta0    = (2, -1),  tau = 30
##
##  Because a(X) multiplies t inside the exponent rather than entering as
##  a multiplicative covariate effect, this is a Gompertz-shape hazard
##  whose SHAPE depends on X, not just its level — the model is not
##  proportional hazards jointly in (X,Z), unlike simulation 01's DGP.
##  A Cox fit here is misspecified for the nuisance effect on purpose.
##
##  Closed-form cumulative hazard, from integrating h(t):
##      H(t|X,Z) = 0.1 exp(theta0'Z) (exp(a(X) t) - 1) / a(X)
##      S(t|X,Z) = exp(-H(t|X,Z))
##  and inverse-transform sampling follows the same closed form as
##  simulation 01: with E ~ Exp(1),
##      T = log(1 + a(X) E / (0.1 exp(theta0'Z))) / a(X).
##
##  A note on the paper's censoring spec: Section 7.1 states "exponential
##  censoring time with mean 1/30" giving "approximately 30%" censoring.
##  Neither reading of that (mean = 1/30, or rate = 1/30 i.e. mean = 30)
##  reproduces 30% under this hazard — simulated at n=200,000 they give
##  99% and 15% respectively. We therefore treat 30% as the design target
##  and calibrate the censoring parameter to it by root-find, exactly as
##  simulation 01 calibrates its own censoring rate.
##
##  Functions are pure: no set.seed inside, so randomness stays under the
##  caller's control.
## =====================================================================

THETA0        <- c(2, -1)
TAU           <- 30
BASE_RATE     <- 0.1   # the leading 0.1 in h(t|X,Z)
A_INTERCEPT   <- 0.1   # the constant term inside a(X) = 0.1 + f(X)^2


## ---------------------------------------------------------------------
##  Covariates
## ---------------------------------------------------------------------

sample_XZ <- function(n) {
  list(X = matrix(runif(3L * n, -1, 1), n, 3L),
       Z = matrix(runif(2L * n, -1, 1), n, 2L))
}

f_nuisance <- function(X) 0.2 * (X[, 1] + X[, 2]) + 0.5 * X[, 1] * X[, 2] + X[, 3]^2
a_shape    <- function(X) A_INTERCEPT + f_nuisance(X)^2


## ---------------------------------------------------------------------
##  Event times by inverse transform
## ---------------------------------------------------------------------

sample_event_times <- function(X, Z, theta = THETA0) {
  aX <- a_shape(X)
  lp <- as.vector(Z %*% theta)
  E  <- -log(runif(nrow(X)))                      # Exp(1)
  log1p(aX * E / (BASE_RATE * exp(lp))) / aX
}

## Ground truth on a grid: m x M. Xtest carries both nuisance and primary
## covariates as named columns (x1..x3, z1..z2); truncation does not
## change this function, since truncation is a sampling mechanism rather
## than a change to the model.
true_survival_matrix <- function(Xtest, tgrid, theta = THETA0) {
  X   <- Xtest[, c("x1", "x2", "x3"), drop = FALSE]
  Z   <- Xtest[, c("z1", "z2"),       drop = FALSE]
  aX  <- a_shape(X)
  lp  <- as.vector(Z %*% theta)
  Ea  <- exp(outer(aX, tgrid))                     # m x M, exp(a_i * t_k)
  H   <- (Ea - 1) / aX * (BASE_RATE * exp(lp))      # recycled row-wise (length m == nrow)
  exp(-H)
}


## ---------------------------------------------------------------------
##  Calibration targets (all analytic, admin censoring at tau folded in)
## ---------------------------------------------------------------------
##  As in simulation 01, each rate integrates out the randomness that has
##  not been drawn yet, given the pilot's T~ values, making calibration a
##  deterministic root-find. A subject with T~ > tau is censored
##  regardless of C, which pmin(Ttilde, tau) captures without a branch:
##  for such a subject the bound saturates at 1 - exp(-tau/theta), which
##  is indistinguishable from "always censored" for any theta << tau.

## P(observed censored) for C ~ Exp(mean = theta), admin cutoff at tau.
## Decreasing in theta.
censoring_rate <- function(theta, Ttilde, tau = TAU) {
  mean(1 - exp(-pmin(Ttilde, tau) / theta))
}

## P(R > T~) for R ~ Unif(cL, cU), independent of T~. Increasing in cU.
trunc_rate <- function(cU, Ttilde, cL) {
  mean(pmin(pmax((cU - Ttilde) / (cU - cL), 0), 1))
}

## P(censored | retained) when C = R + Exp(mean = theta), admin cutoff at
## tau. Decreasing in theta. Valid as long as entry times stay well below
## tau, which holds here since cU is calibrated to a small fraction of
## the follow-up range (checked in the stage 0 sanity checks).
censoring_rate_lt <- function(theta, Ttilde_kept, R_kept, tau = TAU) {
  mean(1 - exp(-(pmin(Ttilde_kept, tau) - R_kept) / theta))
}


## ---------------------------------------------------------------------
##  Samplers
## ---------------------------------------------------------------------

## Right-censored only, no truncation. Not consumed by the two-phase
## Stage 1; defines the untruncated reference DGP and calibrates theta.
make_survival_sample <- function(n, theta, tau = TAU, theta0 = THETA0) {
  XZ     <- sample_XZ(n)
  Ttilde <- sample_event_times(XZ$X, XZ$Z, theta0)
  C      <- rexp(n, rate = 1 / theta)
  data.frame(x1 = XZ$X[, 1], x2 = XZ$X[, 2], x3 = XZ$X[, 3],
             z1 = XZ$Z[, 1], z2 = XZ$Z[, 2],
             time   = pmin(Ttilde, C, tau),
             status = as.integer(Ttilde <= pmin(C, tau)))
}

## Left-truncated, right-censored. Same mechanism as simulation 01: the
## retention filter uses the true event time (truncation is a fact of
## nature), and censoring is residual after entry, C = R + Exp(theta_lt),
## which guarantees entry <= time on every row given R <= cU << tau.
make_ltrc_sample <- function(n, N_pool, theta_lt, cU, cL,
                             tau = TAU, theta0 = THETA0) {
  XZ     <- sample_XZ(N_pool)
  Ttilde <- sample_event_times(XZ$X, XZ$Z, theta0)
  R      <- runif(N_pool, cL, cU)

  keep <- which(R <= Ttilde)
  if (length(keep) < n) {
    stop(sprintf("pool too small: %d retained, need %d", length(keep), n))
  }
  idx <- sample(keep, n)

  Xk <- XZ$X[idx, , drop = FALSE]
  Zk <- XZ$Z[idx, , drop = FALSE]
  Tk <- Ttilde[idx]
  Rk <- R[idx]
  Ck <- Rk + rexp(n, rate = 1 / theta_lt)

  data.frame(x1 = Xk[, 1], x2 = Xk[, 2], x3 = Xk[, 3],
             z1 = Zk[, 1], z2 = Zk[, 2],
             entry  = Rk,
             time   = pmin(Tk, Ck, tau),
             status = as.integer(Tk <= pmin(Ck, tau)))
}


## ---------------------------------------------------------------------
##  The simulation object consumed by the shared engine
## ---------------------------------------------------------------------
##  draw_test samples covariates from the POPULATION distribution, not
##  the retained one — the estimand is the conditional S(t|x,z).

sim <- list(
  name = "02 - Gompertz-shape hazard (paper Sec 7.1), left-truncated right-censored",

  draw_train = function(setup) {
    lt <- setup$ltrc
    make_ltrc_sample(lt$n_ltrc, lt$n_pool, lt$theta_lt, lt$cU, lt$cL,
                     setup$tau, setup$theta0)
  },

  draw_test = function(setup, m) {
    XZ <- sample_XZ(m)
    Xt <- cbind(XZ$X, XZ$Z)
    colnames(Xt) <- c("x1", "x2", "x3", "z1", "z2")
    Xt
  },

  truth = function(Xtest, tgrid, setup) {
    true_survival_matrix(Xtest, tgrid, setup$theta0)
  }
)
