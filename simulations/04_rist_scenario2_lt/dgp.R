## =====================================================================
##  DGP — RIST paper, Scenario 2 (p. 334), with left truncation added
## =====================================================================
##  Source: Zhu and Kosorok (2012), "Recursively Imputed Survival Trees,"
##  JASA 107(497), 331-340, Section 4.1, Scenario 2
##  (../../papers/Recursively Imputed Survival Trees.pdf).
##
##  As printed: "We draw 10 iid uniform distributed covariates and use link
##  function mu = sin(x1*pi) + 2*|x2-0.5| + x3^3 to create a violation of
##  the proportional hazards assumption. Survival times follow an
##  exponential distribution with mean mu. Censoring times are drawn
##  uniformly from (0, tau), where tau = 6."
##
##  Only x1, x2, x3 drive mu; x4..x10 are pure noise covariates, unlike
##  Scenario 1 the covariates are iid (no correlation structure), and mu
##  enters the hazard nonlinearly and non-additively -- the deliberate
##  proportional-hazards violation this scenario is designed to test.
##
##  Two things not stated in the paper, resolved here:
##    - The covariate range. "iid uniform" without bounds is read as
##      Unif(0,1), the standard default and consistent with |x2-0.5| being
##      centered where you'd expect.
##    - No separate administrative cutoff is used: the censoring
##      distribution's own upper bound already caps every observed time,
##      so unlike simulations 01-03 there is no extra pmin(., tau).
##
##  What is taken from the paper, unchanged:
##    p = 10, X ~ iid Unif(0,1)
##    mu(X) = sin(x1*pi) + 2*|x2-0.5| + x3^3
##    T ~ Exponential(mean = mu(X))
##    C ~ Uniform(0, U), U a single shared upper bound (not per-subject)
##    target censoring rate ~24%
##
##  What we add, not in the paper (Scenario 2 has no left truncation):
##    entry time R ~ Unif(cL, cU), retention filtered on R <= Ttilde,
##    same mechanism as simulations 01-03. Unlike those simulations, the
##    residual censoring after entry keeps the paper's own UNIFORM shape
##    rather than switching to Exponential: C = R + Unif(0, w_lt). Using
##    Exponential here would silently erase the one thing that
##    distinguishes Scenario 2's censoring design from Scenario 1's.
##    cU, U (reference), and w_lt are all calibrated by root-find against
##    a target rate, following this project's established pattern -- the
##    paper's ~24% is treated as the design target for both.
##
##  Functions are pure: no set.seed inside, so randomness stays under the
##  caller's control.
## =====================================================================

P      <- 10L
ACTIVE <- 1:3   # x1, x2, x3 drive mu; x4..x10 are noise


## ---------------------------------------------------------------------
##  Covariates
## ---------------------------------------------------------------------

sample_X <- function(n, p = P) {
  matrix(runif(n * p, 0, 1), n, p)
}

## The nonlinear, non-additive link that violates proportional hazards:
## sin() bends the effect of x1, |.-0.5| folds x2 around its midpoint, and
## x3^3 is a convex effect of x3. None of these is a simple multiplicative
## covariate effect, unlike simulations 01 and 03.
mu_link <- function(X) {
  x1 <- X[, 1]; x2 <- X[, 2]; x3 <- X[, 3]
  sin(x1 * pi) + 2 * abs(x2 - 0.5) + x3^3
}


## ---------------------------------------------------------------------
##  Event times by inverse transform
## ---------------------------------------------------------------------
##  T ~ Exponential(mean = mu(X)). S(Ttilde|X) ~ U(0,1), so solving
##  S(Ttilde|X) = U gives Ttilde = -mu(X) * log(U), the same closed form
##  used in simulations 01 and 03.

sample_event_times <- function(X) {
  mu <- mu_link(X)
  U  <- runif(nrow(X))
  -mu * log(U)
}

## Ground truth on a grid: m x M. S(t|X) = exp(-t / mu(X)). Unchanged by
## truncation, which is a sampling mechanism rather than a change to the
## model. Xtest must carry columns x1..xP (only x1..x3 are read).
true_survival_matrix <- function(Xtest, tgrid) {
  mu <- mu_link(Xtest)
  exp(-outer(1 / mu, tgrid))
}


## ---------------------------------------------------------------------
##  Calibration targets (all analytic)
## ---------------------------------------------------------------------
##  Each rate integrates out the randomness that has not been drawn yet,
##  given the pilot's Ttilde values, making calibration a deterministic
##  root-find.

## P(observed censored) for C ~ Unif(0, U), independent of Ttilde. For
## C ~ Unif(0,U), P(C < t) = min(t, U) / U. Decreasing in U: a wider
## censoring window pushes C higher on average, so it is less likely to
## land below Ttilde.
censoring_rate <- function(U, Ttilde) {
  mean(pmin(Ttilde, U) / U)
}

## P(R > Ttilde) for R ~ Unif(cL, cU), independent of Ttilde. Increasing
## in cU.
trunc_rate <- function(cU, Ttilde, cL) {
  mean(pmin(pmax((cU - Ttilde) / (cU - cL), 0), 1))
}

## P(censored | retained) when C = R + Unif(0, w), w a shared width.
## Censored iff the residual wait undercuts the remaining lifetime,
## W < Ttilde - R; for W ~ Unif(0,w), P(W < d) = min(d, w) / w. Decreasing
## in w for the same reason as censoring_rate() above.
censoring_rate_lt <- function(w, Ttilde_kept, R_kept) {
  d <- Ttilde_kept - R_kept
  mean(pmin(d, w) / w)
}


## ---------------------------------------------------------------------
##  Samplers
## ---------------------------------------------------------------------

## Right-censored only, no truncation. Not consumed by the two-phase
## Stage 1; retained because it defines the untruncated reference DGP and
## calibrates the censoring upper bound U. Unlike RIST Sc. 2's own fixed
## U = tau = 6, U here is root-found against TARGET_CENSOR in
## stage0_setup.Rmd -- see the note at the top of this file.
make_survival_sample <- function(n, cens_upper) {
  X      <- sample_X(n)
  Ttilde <- sample_event_times(X)
  C      <- runif(n, 0, cens_upper)
  colnames(X) <- paste0("x", seq_len(ncol(X)))
  data.frame(X, time = pmin(Ttilde, C), status = as.integer(Ttilde <= C))
}

## Left-truncated, right-censored. Same mechanism as simulations 01-03:
## the retention filter uses the true event time, and censoring is
## residual after entry -- here C = R + Unif(0, w_lt), keeping the
## paper's own Uniform shape rather than switching to Exponential -- which
## guarantees entry <= time on every row.
make_ltrc_sample <- function(n, N_pool, w_lt, cU, cL) {
  X      <- sample_X(N_pool)
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
             time   = pmin(Tk, Ck),
             status = as.integer(Tk <= Ck))
}


## ---------------------------------------------------------------------
##  The simulation object consumed by the shared engine
## ---------------------------------------------------------------------
##  draw_test samples covariates from the POPULATION distribution, not the
##  retained one -- the estimand is the conditional S(t|x).

sim <- list(
  name = "04 - RIST Scenario 2 DGP (p=10, nonlinear PH-violating link), left-truncated right-censored",

  draw_train = function(setup) {
    lt <- setup$ltrc
    make_ltrc_sample(lt$n_ltrc, lt$n_pool, lt$w_lt, lt$cU, lt$cL)
  },

  draw_test = function(setup, m) sample_X(m),

  truth = function(Xtest, tgrid, setup) true_survival_matrix(Xtest, tgrid)
)
