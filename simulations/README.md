# Simulations overview

Six simulations, all answering the same question — does giving a model the
true entry time help, when data is left-truncated? — on six different
data-generating processes (DGPs). Every folder shares the same two-stage
structure (`stage0_setup.Rmd` calibrates and saves `setup.rds`,
`stage1_main.Rmd` runs the two-phase experiment) and the same engine
(`_common/sim_engine.R`, `_common/sim_report.R`). Only `dgp.R` differs
between them.

Left truncation itself is **never** part of any source paper — it is added
the same way in all six: entry time $R\sim\mathrm{Unif}(c_L,c_U)$
independent of $(\tilde T,X)$, a subject retained only if it survived to
its own entry ($R\le\tilde T$), and censoring redrawn as a *residual*
duration after entry so `entry <= time` holds on every row. What differs
per DGP is only the *shape* of that residual censoring draw (see table).

## The six DGPs

| # | Folder | Source | Event time $T\mid X$ | True $S(t\mid X)$ | True hazard $h(t\mid X)$ | Proportional hazards? |
|---|---|---|---|---|---|---|
| 01 | `01_cox_random_forerst_no_lt` | Ishwaran et al. (2010)-style Cox PH | Weibull(shape 2), scale via $X$ | $\exp\!\big(-A_0 t^2 e^{\beta'X}\big)$, $A_0=0.005$ | $0.01\,t\,e^{\beta'X}$ | **Yes** — exactly Cox by construction |
| 02 | `02_flexihaz_gompertz_lt` | "Flexible Deep NNs for Partially Linear Survival Data," Sec. 7.1 | Gompertz-shape | $\exp\!\Big(\!-0.1e^{\theta_0'Z}\frac{e^{a(X)t}-1}{a(X)}\Big)$ | $0.1\,e^{a(X)t+\theta_0'Z}$, $a(X)=0.1+f(X)^2$ | **No** — $a(X)$ multiplies $t$ in the exponent, so the hazard's *shape over time*, not just its level, depends on $X$. Genuine, textbook non-proportionality. |
| 03 | `03_rist_scenario1_lt` | RIST paper, Scenario 1 (p.334) | Exponential | $\exp\!\big(-t\,e^{-\beta'X}\big)$ | $e^{-\beta'X}$ (constant in $t$) | **Yes** — exactly Cox; $\beta'X$ enters linearly |
| 04 | `04_rist_scenario2_lt` | RIST paper, Scenario 2 (p.334) | Exponential | $\exp\!\big(-t/\mu(X)\big)$, $\mu(X)=\sin(x_1\pi)+2\|x_2-0.5\|+x_3^3$ | $1/\mu(X)$ (constant in $t$) | **Nominal violation only** — each subject's own hazard *is* constant in $t$, so the hazard ratio between any two subjects is trivially constant over time too. What actually breaks Cox is that $\mu(X)$ is a bent/folded/curved function of $X$ that a linear log-hazard cannot represent — a wrong-functional-form problem, not a hazard-changes-shape-over-time problem. See simulation 04's discussion with the user for the full distinction from 02. |
| 05 | `05_rist_scenario3_lt` | RIST paper, Scenario 3 (p.334) | Gamma(shape $=\mu(X)$, scale $=2$) | $1-F_{\mathrm{Gamma}}(t;\mu(X),2)$ — no elementary closed form (incomplete gamma) | $f_{\mathrm{Gamma}}(t;\mu(X),2)/S(t\mid X)$ — no simple closed form | **No** — Gamma hazard shape is genuinely governed by its shape parameter $\mu(X)$ (decreasing for $\mu<1$, increasing for $\mu>1$), so it varies with $t$ *and* that variation itself depends on $X$ |
| 06 | `06_rist_scenario4_lt` | RIST paper, Scenario 4 (p.334) | Lognormal(meanlog $=\mu(X)$, sdlog $=1$) | $1-\Phi\!\big((\ln t-\mu(X))/1\big)$ | non-monotonic in $t$ (rises then falls); no simple closed form | **No** — Lognormal hazard is famously non-monotonic and its shape depends on $\mu(X)$, so it is neither constant nor does it scale proportionally across subjects |

$\beta'X$, $\mu(X)$, $a(X)$, $f(X)$ are each defined per-DGP — see the
"active covariates" column below and each `dgp.R` for the exact formula.

## Everything else worth knowing, per simulation

| # | $p$ (active of $p$) | Covariate correlation | Censoring shape (residual, after entry) | Admin cutoff $\tau$ | Notable finding |
|---|---|---|---|---|---|
| 01 | 6 (6 active) | AR(1), $\rho=0.2$ | Exponential | none | Baseline of the whole project; simplest exactly-Cox case |
| 02 | 5 (5 active: 3 nuisance + 2 primary) | none (independent) | Exponential | 30 | Cox misspecified for the nuisance nonlinearity by construction |
| 03 | 25 (10 active) | AR(1), $\rho=0.9$ | Exponential | 4 | The printed paper formula ($\mu=b_0\sum x_i$) is mean-zero and invalid for an Exponential mean; corrected to $\mu=\exp(b_0\sum x_i)$ by checking the paper's own cited source (Ishwaran et al. 2010) |
| 04 | 10 (3 active) | none (iid) | Uniform | 6 (via the censoring window itself, no separate cutoff) | Covariate range not stated by the paper; assumed $\mathrm{Unif}(0,1)$ |
| 05 | 25 (5 active) | AR(1), $\rho=0.75$ | Uniform | 10 | Censoring window (1.5$\tau$=15) is *wider* than $\tau$, so admin- and window-censoring are structurally distinct cases — handled with an exact branching formula rather than the single-expression approximation used in 01–03 |
| 06 | 25 (10 active: two disjoint groups of 5) | AR(1), $\rho=0.75$ | Lognormal, **X-dependent** (shares $\mu(X)$ with $T$, shifted) | 4 | Spread (`sdlog`) not stated by the paper; assumed 1. Truncation's own survivorship enrichment pushes the retained population's admin-censoring floor to ~37%, above the paper's ~32% target — confirmed mathematically unreachable, so the extra censoring window is disabled and the achieved rate is reported honestly instead |

## Reading the "proportional hazards?" column

Two distinct ways a DGP can break Cox show up across these six:

1. **The hazard's shape over time changes with $X$** (simulations 02, 05,
   06) — the textbook definition of non-proportional hazards: the ratio of
   two subjects' hazards is not constant over $t$.
2. **The hazard is constant over time, but the covariate effect on its
   level is not linear** (simulation 04) — Cox is still misspecified (it
   can only fit a linear combination of covariates), but not because of
   time-varying hazard ratios; those are trivially constant here since
   neither subject's hazard depends on $t$ at all.

Simulations 01 and 03 are the two "control" cases where Cox is exactly
correct — one low-dimensional and uncorrelated, one high-dimensional and
strongly correlated — so any degradation seen elsewhere can be attributed
to the specific violation each DGP introduces, not to dimensionality or
correlation alone.

## Results, as knitted

Pulled directly from each `stage1_main.html`'s phase table (score = mean
absolute error vs. the true $S(t\mid x)$; lower is better). "Shift" is
phase 1 (entry zeroed) minus phase 2 (entry supplied) — negative means
supplying entry times helped.

| # | `N_REP` | Cox: standard → entry-supplied | Cox improvement | RSF: standard → entry-supplied | RSF improvement | Phase-1 no-op check |
|---|---|---|---|---|---|---|
| 01 | 100 | 0.1110 → 0.0242 | **78%** | 0.1141 → 0.0401 | **65%** | exact (Cox gap 1e-14, RSF gap 0) |
| 02 | 20 | 0.1214 → 0.1038 | 15% | 0.1134 → 0.0789 | 30% | exact |
| 03 | 10 | 0.0995 → 0.0185 | **81%** | 0.1016 → 0.0651 | 36% | exact |
| 04 | 10 | 0.1509 → 0.0852 | 44% | 0.1338 → 0.0773 | 42% | exact |
| 05 | 10 | 0.1995 → 0.1524 | 24% | 0.1280 → 0.0864 | 32% | exact |
| 06 | 10 | 0.1767 → 0.1004 | 43% | 0.1458 → 0.0807 | 45% | exact |

Every one of the twelve within-arm shifts (six sims × two arms) is a real
improvement, and in every simulation except 02's Cox arm (19/20
replications), *every single replication* moved in the improving direction
— not just the average. The phase-1 no-op check (standard fit vs.
entry-zeroed left-truncation fit, which should be identical) passes exactly
in all six: Cox agrees to ~1e-14 against a 1e-8 tolerance, RSF agrees to
machine-zero exactly, confirming the entry-time argument is truly inert
when there is no entry information to use.

Two patterns worth noting:

- **The two exactly-Cox DGPs (01, 03) show Cox's own improvement (78%,
  81%) clearly outpacing RSF's (65%, 36%)** — when Cox is the right model,
  correcting its risk set for left truncation removes bias cleanly, more
  so than it does for a nonparametric method that has to relearn the risk
  set's shape from fewer effective observations.
- **In every PH-violating DGP (02, 04, 05, 06), RSF's improvement is
  comparable to or larger than Cox's** (30% vs 15% in 02; 42% vs 44% in
  04; 32% vs 24% in 05; 45% vs 43% in 06) — consistent with Cox carrying a
  misspecification penalty on top of the truncation bias, which the
  entry-time correction alone cannot fix.
