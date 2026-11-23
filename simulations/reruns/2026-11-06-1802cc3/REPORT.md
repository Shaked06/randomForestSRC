# Refreshed simulation results

Package/source commit: `1802cc3`.
Completed: 2026-11-07 04:24:26 UTC.

The six scenarios use their preserved setup files and original seeds, with 5,000 training subjects, 500 trees per forest, and 200 population test covariates per replication. No hyperparameters were retuned. Both phases use the same data within each replication.

Every replication independently checks ordinary versus zero-entry Cox/RSF fits. These checks pass. The package implementation is fingerprinted in each saved result, and workers verify the matching installation.

This is a refresh after the bug fixes and upstream integration. Historical results lack package provenance. Changes from historical numbers can reflect corrected scoring, the Cox lookup fix, prediction/restoration fixes, and upstream censoring/RNG changes; the comparison does not isolate individual fixes.

MAD evaluates unconditioned survival curves against source-population truth. The C-index, entered-cohort Brier/CRPS, and AUC use training outcomes (RSF OOB curves; Cox in-sample curves). The simulation C-index helper is unweighted; Brier/AUC use the pooled delayed-entry Kaplan-Meier censoring model. These metrics assess different targets. The pooled model assumes a common censoring hazard; scenarios with censoring depending on covariates/entry require care in interpreting these descriptive scores. AUC is our extension, not a prescribed estimator in the professor's outline.

The original saved simulations and pending research edits remain untouched. Fresh result objects are under `results/<scenario>/results_phases.rds`; `summary.csv` contains paired phase shifts and descriptive 95% normal-approximation intervals, while `comparison.csv` records historical/current means. The six original stage1_main.html reports were refreshed from these 100-replication results on 7 November 2026; previous HTML copies are preserved under source/.

## Run configuration

|scenario                 | n_train| ntree| mtry| nodesize| n_rep| m_test|
|:------------------------|-------:|-----:|----:|--------:|-----:|------:|
|01_cox_random_forerst_lt |    5000|   500|    3|       16|   100|    200|
|02_flexihaz_gompertz_lt  |    5000|   500|    3|       16|   100|    200|
|03_rist_scenario1_lt     |    5000|   500|    5|        6|   100|    200|
|04_rist_scenario2_lt     |    5000|   500|    3|        6|   100|    200|
|05_rist_scenario3_lt     |    5000|   500|    5|        6|   100|    200|
|06_rist_scenario4_lt     |    5000|   500|    5|        6|   100|    200|

## Results




Thin summary of all six simulations' knitted results. For the DGP math, true
`S`/hazard, and PH verdicts, see the [simulation overview](../../README.md) — this file is just the numbers,
computed directly from each sim's `results_phases.rds` (nothing here is
hand-typed).

## What "phase 1 → phase 2" means

Every simulation fits the *same* entry-time-aware model (`cox_lt`/`rsf_lt`) on
the *same* replication twice:

- **Phase 1**: that model is given an entry vector of all zeros — i.e.
  "pretend nothing is truncated."
- **Phase 2**: that same model is given the real, observed entry times.

So every number below is a **paired before/after comparison on one model**,
not a comparison against a different (standard) model. It happens that phase
1 (entry = 0) is mathematically proven identical to the plain/standard
model's own score (the "noop" check each sim's report also shows), so in
practice this also tells you standard-vs-entry-aware — but the actual
quantity being computed is: *does giving this model the real entry time
improve it, relative to giving it no truncation information at all?*



> **Run size:** 100 replications per simulation,
> 200 test subjects each.
> 

Every column is **% improvement from supplying the real entry time**
(`(phase1 - phase2) / phase1 * 100`), computed on each metric's *error*
scale. So for every column alike, positive = entry time helped, negative =
it made that arm/metric worse.

MAD and Brier are lower-is-better as they stand. The two ranking metrics are
converted to errors first so they sit on that same scale: the C-index column
is computed on `1 - C`, and the AUC column on `1 - AUC`. Read an AUC entry
of `2.9%` as *"the AUC error shrank by 2.9%"* — not as an AUC rising by 2.9
points.


|#  |DGP                                 |MAD Cox |MAD RSF |C-idx Cox |C-idx RSF |Brier Cox |Brier RSF |AUC Cox |AUC RSF |
|:--|:-----------------------------------|:-------|:-------|:---------|:---------|:---------|:---------|:-------|:-------|
|01 |Cox PH (exactly Cox)                |91.39%  |66.01%  |0.03%     |-0.38%    |5.96%     |5.48%     |0.95%   |1.45%   |
|02 |Gompertz-shape (PH violation)       |16.66%  |36.23%  |0.01%     |-1.03%    |3.84%     |2.93%     |0.58%   |0.30%   |
|03 |RIST Sc.1 (exactly Cox)             |81.53%  |36.42%  |0.03%     |-0.02%    |3.29%     |1.74%     |1.42%   |1.20%   |
|04 |RIST Sc.2 (nonlinear link)          |46.30%  |41.45%  |0.03%     |0.04%     |4.20%     |2.51%     |1.51%   |1.96%   |
|05 |RIST Sc.3 (Gamma, PH violation)     |24.46%  |31.05%  |0.15%     |0.46%     |4.20%     |2.49%     |1.32%   |1.45%   |
|06 |RIST Sc.4 (Lognormal, PH violation) |48.43%  |44.11%  |0.27%     |0.66%     |7.00%     |4.29%     |1.02%   |2.45%   |

## By metric



Same numbers as the table above, as 4 small multiples (one
axis each — MAD, C-idx, Brier, and AUC live on very different
scales, so one shared axis would be misleading). The zero line is the
reference: bars crossing below it are where entry time made that arm/metric
*worse* — in this run, C-idx RSF.

![plot of chunk plot-mad](results/RESULTS_files/figure-gfm/plot-mad-1.png)

![plot of chunk plot-cindex](results/RESULTS_files/figure-gfm/plot-cindex-1.png)

![plot of chunk plot-brier](results/RESULTS_files/figure-gfm/plot-brier-1.png)

![plot of chunk plot-auc](results/RESULTS_files/figure-gfm/plot-auc-1.png)

Raw phase-table numbers (all 4 metrics, all six sims) are in
each sim's `results_phases.rds` / knitted `stage1_main.html`. The AUC also has
a by-time curve there, plotted per simulation — the single number above is that
curve time-averaged, which can flatten differences that are concentrated in
one stretch of follow-up.
