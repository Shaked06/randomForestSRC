# Simulations overview

Six simulations, all answering the same question — does giving a model the
true entry time help, when data is left-truncated? — on six different
data-generating processes (DGPs). Every folder shares the same two-stage
structure (`stage0_setup.Rmd` calibrates and saves `setup.rds`,
`stage1_main.Rmd` renders or runs the two-phase experiment) and the same engine
(`_common/sim_engine.R`, `_common/sim_report.R`). Only `dgp.R` differs
between them.

## Rendering the completed run

Knitting any `stage1_main.Rmd` now defaults to loading the completed
**100 replications per experiment** from `reruns/2026-11-06-1802cc3`.
It checks the saved run metadata and regenerates the HTML's tables and plots
without fitting models or changing any saved results. The YAML
`results_run` parameter selects the run, relative to that scenario's folder.

To render all six reports and the HTML/Markdown results summary from the repository root:

```sh
Rscript simulations/reruns/2026-11-06-1802cc3/render_rmd.R
```

For a new fitting run, explicitly set `recompute: true` and choose `n_rep`,
`m_test`, and `n_cores`. That mode requires the installed LT-enabled package
and writes the scenario's local `results_phases.rds`. Leave `recompute: false`
to retain the completed-run reporting workflow.

Stage 1 caches include fingerprints of the installed R code and compiled
libraries for `randomForestSRC` and `survival`, plus their installation paths,
R version, and platform. A same-version package reinstall can therefore
invalidate the cache. New `results_phases.rds` objects record this information
in `package_provenance`, and workers verify that they use the same packages.
Editing the source checkout does not update an installed package: reinstall
the package and restart R before knitting after a code change. Existing saved
results without this field are historical outputs; this change does not rerun
or relabel them. Fingerprints cover these two modeling packages, not every
dependency, system library, compiler setting, or in-memory namespace edit.
This protects Stage 1 replication caches. Stage 0 calibration/tuning caches
are unchanged; clear those caches explicitly when retuning after a package
change. Stage 1 still hashes the selected `setup.rds` as an explicit input.

Left truncation itself is **never** part of any source paper — it is added
the same way in all six: entry time $R\sim\mathrm{Unif}(c_L,c_U)$
independent of $(\tilde T,X)$, a subject retained only if it survived to
its own entry ($R\le\tilde T$), and censoring redrawn as a *residual*
duration after entry so `entry <= time` holds on every row. What differs
per DGP is only the *shape* of that residual censoring draw (see table).

## The six DGPs

| # | Folder | Source | Event time $T\mid X$ | True $S(t\mid X)$ | True hazard $h(t\mid X)$ | Proportional hazards? |
|---|---|---|---|---|---|---|
| 01 | `01_cox_random_forerst_lt` | Ishwaran et al. (2010)-style Cox PH | Weibull(shape 2), scale via $X$ | $\exp\!\big(-A_0 t^2 e^{\beta'X}\big)$, $A_0=0.005$ | $0.01\,t\,e^{\beta'X}$ | **Yes** — exactly Cox by construction |
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

## Current results

The current [results summary](RESULTS.md) and
[HTML summary](RESULTS.html) use the same completed 100-replication run as
all six `stage1_main.html` reports. They include MAD, concordance error,
Brier/CRPS, and AUC error, with paired uncertainty and interpretation limits.

MAD improves in all six scenarios for both model families. Concordance
changes are mixed; see the summary for the scenario-specific results.
Earlier saved result files remain historical and are not the default source
for the reports.
