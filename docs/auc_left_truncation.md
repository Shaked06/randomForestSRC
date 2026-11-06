# Time-dependent AUC with delayed entry

## Target and relation to the professor's outline

`get.auc.survival()` evaluates cumulative/dynamic discrimination at each
prediction time. Higher AUC is better: 1 is perfect ranking, and equal
markers receive half credit. The separate exported `get.auc()` evaluates
classification rather than survival.

The professor's project outline, section 9, specifies delayed-entry
concordance and Brier evaluation. It does not specify an AUC estimator.
Our AUC is an additional project metric, using the same enrolled-subject,
entry-conditioned interpretation documented for Brier in
`docs/brier_score_left_truncation.md`. The professor should review this
extension before it is treated as settled in the thesis.

This evaluates ranking **among enrolled subjects after entry**. It is not
an estimator of source-population AUC from time zero. That distinction is
substantive: source-population methods require a justified correction for
truncation and estimation of different nuisance distributions. For an
example of that separate approach, see Li and Acharjee, sections 2 and 3.2:
https://arxiv.org/html/2509.05693v1. Our estimator below is a project
derivation, rather than their proposed source-population estimator.

## Entered-cohort estimator

Write `U_i = min(T_i,C_i)`, with `delta_i = 1` for an observed failure.
At evaluation time `t`, subjects must have entered (`L_i <= t`). Cases
have `U_i <= t, delta_i = 1`; controls have `U_i > t`. Subjects censored
by `t` have unknown case/control membership and are handled through IPCW.
This entered-subject set includes earlier observed failures; it is not
just the set still at risk of failure at `t`.

The marker is conditional event risk from the subject's own entry:

```text
M_i(t) = 1 - S_hat(t | X_i) / S_hat(L_i^- | X_i).
```

The left limit respects inclusive entry (`T_i >= L_i`), so a failure
exactly at entry remains possible. Conditioning can change the ranking
between subjects with different entry times.

Censoring survival `G_i` is estimated using delayed-entry risk sets.
Observation weights conditional on reaching entry are

```text
w_case_i(t) = G_i(L_i^-)/G_i(U_i^-)
w_control_j(t) = G_j(L_j^-)/G_j(t).
```

Let `h(a,b) = I(a>b) + 0.5 I(a=b)`. Then

```text
AUC_entry(t) = sum_case_i sum_control_j w_case_i w_control_j h(M_i,M_j)
              / (sum_case_i w_case_i * sum_control_j w_control_j).
```

With no censoring, every weight is one and this is plain pairwise ranking
among entered subjects. No entry-distribution weights are included.

The shared censoring implementation is identical to Brier's: the `km`
option uses product-limit Kaplan–Meier survival with delayed-entry risk
sets and failures removed before censoring at tied times; the `rfsrc` option fits a random-split censoring forest using the
covariates and entry, with delayed-entry risk sets and predictor imputation.
Pooled `km` requires a common censoring hazard. More generally, independent
censoring given the censoring-model information and positive observation
probabilities over the evaluation horizon are required. The forest option
is model-based, not a guarantee of valid censoring adjustment.

## Why the old entry weights were removed

The retained entry times do not generally have the source entry distribution.
For independent source `L,T ~ Uniform(0,1)`, retaining `L<T` gives observed
entry CDF `2l-l^2`. At `l=.5`, the observed CDF is `.75`, although the source
CDF is `.5`. Treating one as the other is an inconsistency even in an
arbitrarily large sample.

Replacing that empirical CDF with a source-CDF estimator would still require
justifying a source-population AUC target. Our entered-cohort target does
not require that distribution at all, matching the Brier decision in fix 5.

An exact uncensored example uses independent source `T` uniform on `{1,2,3}`
and `L` uniform on `{0,1.5}`. Five equally likely combinations satisfy
`L <= T`. At `t=2`, true conditional markers are `2/3` for early entrants
and `1/2` for late entrants. Direct comparison of three cases with two
controls gives `7/12 = 0.5833333`. The old entry weights instead give
`27/44 = 0.6136364`. The corrected implementation matches the direct result.

## API, computation, and limitations

Supply `entry.time` in full evaluation-row order before scoring `subset`.
For new-data prediction objects, censoring is fitted using the saved training
entries and outcomes, separately from validation entries and outcomes.
Selected outcomes, predictions, and entries are aligned. Predicted survival
must be valid for conditioning; zero survival at entry produces an error.
Before-entry and missing OOB predictions are excluded from comparisons.
A time with no usable cases or controls returns `NA`. Zero or undefined
required censoring probabilities produce an error asking for a supported
horizon; cases are not silently discarded for zero weights.

`entry.time = NULL` uses the updated upstream right-censored path. Explicit
zero entry uses the updated ordinary IPCW estimator on supported horizons.
Historical numerical results can change with the updated censoring estimator
and RNG protocol; the uncensored oracle examples retain their expected values.

The implementation sorts control markers and uses cumulative weights,
retaining half credit for ties without constructing a quadratic pair matrix.
Groupwise normalization of log weights avoids overflow without changing
the AUC ratio.

The output retains `auc`, `iauc`, `cens.dist`, `time`, `event.info`, `subset`,
`mort`, and `surv.ensb`. The latter contains conditioned curves with `NA`
before entry. `GL` is `NULL`; `score.type` names the target, and
`censoring.at.entry` records the entry numerators.

`iauc` retains its historical descriptive time average: trapezoidal
integration over defined grid points divided by their time span. It can
bridge interior undefined points. This is not an event-distribution-weighted
population integrated AUC.

Bug 7 is addressed in the integration by using upstream's separate grow
and evaluation outcomes, for both the ordinary and LT paths. Unlabeled
prediction objects raise a clear error. The upstream name
`get.auct.survival()` is retained alongside our compatibility name
`get.auc.survival()`. Both accept `entry.time` and `times`; the compatibility
name retains the `auc`/`iauc` output used by thesis simulations. LT confidence
intervals are explicitly unsupported. Historical simulation outputs have
not been regenerated.

## Regression tests

`tests/entry-time-auc.R` fails on the old implementation and checks the exact
five-subject counterexample, an explicit weighted case-by-control reference,
hand-computed delayed-entry censoring, event/censor ties, inclusive entry,
no entrants/cases/controls, one-point grids, subset permutations, and missing
OOB predictions. Random small fixtures compare against independently
estimated product-limit survival from `survival::survfit` and quadratic pair
matrices. The forest-censoring check also verifies agreement with Brier's
censoring curves under the same RNG seed, including missing predictors.
An actual delayed-entry forest's OOB AUC is checked against the independent
pairwise reference as well.
