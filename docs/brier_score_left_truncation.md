# Brier score with delayed entry

## Governing outline and interpretation

The source of truth is the professor's project outline,
`docs/thesis/left_truncated_RSF_student_project.pdf`, section 9.2, page 7.
It specifies squared prediction error, conditioning on being under
observation, restricting evaluation to subjects with entry `L_i <= t`,
and censoring weights estimated using delayed-entry risk sets.

The outline does **not** give a fully expanded estimator. The following
entry-conditioned formula is our interpretation of its observation
conditioning, rather than an equation quoted from the PDF. The professor
should review this interpretation before the thesis treats it as settled.

This score evaluates prediction **after entry for enrolled subjects**.
It does not estimate prediction error from time zero for the unobserved
source population. In particular, it differs in target from the
simulation's source-population truth-based MAD.

## Predictions and score

Let `U_i = min(T_i, C_i)` and `delta_i` indicate an observed failure.
The forest estimates survival from time zero, `S_hat(t | X_i)`.
With inclusive entry `T_i >= L_i`, its entry-conditioned prediction is

```text
q_i(t) = S_hat(t | X_i) / S_hat(L_i^- | X_i),     t >= L_i.
```

The left limit matters when entry and an event time coincide: a failure
exactly at entry remains possible. The stored prediction is a step function
on the forest's time grid, with value 1 before its first point.

Let `G_i(u)` denote censoring survival from the original time origin,
conditional on the covariates and entry information required by the
censoring model. Observation probabilities after entry are ratios to
`G_i(L_i^-)`. At time `t` the implemented score is

```text
BS_entry(t) = 1/n_t * sum over i with L_i <= t of
  [ delta_i * I(U_i <= t) * q_i(t)^2 * G_i(L_i^-)/G_i(U_i^-)
    + I(U_i > t) * (1-q_i(t))^2 * G_i(L_i^-)/G_i(t) ].
```

Here `n_t` counts selected, entered subjects with a finite prediction.
A subject censored by `t` contributes zero but stays in that denominator;
IPCW accounts for the unavailable outcome. A subject not entered by `t`
is excluded altogether. With no censoring this reduces to ordinary mean
squared error of the conditional probabilities among entered subjects.

No inverse entry-CDF weights are included. The old combination of such
weights, entry-conditioned predictions, and entered-subject averaging
could favor an incorrect conditional probability even without censoring.
Also, the observed entry CDF is generally not the source entry CDF after
selection. This correction avoids needing either CDF for this score.

## Censoring models and assumptions

The conditional IPCW argument requires independent failure and censoring
given the information used by the censoring model, plus positive observation
probabilities over the evaluation horizon. These are modeling assumptions,
not properties guaranteed by the implementation.

With `cens.model = "km"`, one pooled censoring hazard is estimated at each
actual censoring time `u` using

```text
Y_C(u) = sum I(L_i <= u <= U_i)
d_C(u) = sum I(U_i = u, delta_i = 0)
G_hat(u) = exp(-sum over v <= u of d_C(v)/Y_C(v)).
```

Despite the historical option name `km`, the package uses exponential
Nelson–Aalen cumulative hazard rather than a product-limit estimator.
We retain that estimator and change its risk sets to account for entry.
The pooled option assumes a common censoring hazard; conditional independence
only given `X,L`, as permitted by the outline, does not establish that
stronger assumption. Interpret scores accordingly when censoring varies
with covariates or entry.

With `cens.model = "rfsrc"`, a separate random-split censoring forest uses
the original covariates and entry as predictors, with the same delayed-entry
risk-set support used to fit the outcome forest. Its censoring survival
curves provide subject-specific ratios. This is a model-based estimate,
and a correctly specified censoring model remains necessary. Missing
predictors use imputation so subject rows are preserved. With no censoring,
both options use weights of one without fitting a censoring forest.

Both options evaluate event weights at `G(U_i^-)`, survival weights at
`G(t)`, and the entry numerator at `G(L_i^-)`, using actual times even
when the scoring grid omits a censoring time. Risk sets follow the outline's
inclusive entry convention. Ties between entry and exit are allowed.

## API and boundaries

Supply `entry.time` in the full training-row order, including when a subset
is scored. The LT path aligns selected outcomes, entries, and predictions.
Entry must be finite, nonnegative, and no later than follow-up; this scorer
requires known follow-up and binary event status.

`entry.time = NULL` retains the existing ordinary Brier implementation.
Explicit zero entry reduces the new estimator to ordinary IPCW conceptually.
Exact numeric agreement with the historical implementation is not guaranteed
for every coarse grid or tie configuration because the LT path evaluates
weights at their actual times and uses explicit left limits.

If no subject has entered, the score is undefined (`NA`), rather than zero.
CRPS is also `NA` if any score on its integration grid is undefined; it does
not silently bridge unsupported intervals. Zero predicted survival at entry
or zero/undefined required censoring survival produces an informative error.
Subjects without finite OOB predictions are omitted at the relevant time.

Existing output fields are retained. `surv.ensb` contains the original curves;
`conditional.survival` contains the entry-conditioned curves, with `NA`
before entry. `n.eligible` records the scoring denominator, and
`censoring.at.entry` contains the entry numerators. `GL` and the legacy
reference overlay `surv.aalen` are `NULL` for LT scoring; no comparable
reference overlay is supplied by this implementation.

The inherited prediction-object outcome issue (bug 7) remains open: shared
setup still uses training outcomes. Do not treat this change as validation
of scoring new-data prediction objects. Ordinary Brier subsetting (bug 8)
and AUC's entry weights/ties (bugs 6 and 9) also remain separate issues.

## Regression checks

`tests/entry-time-brier.R` checks hand-computed uncensored and censored
examples, delayed-entry risk sets, off-grid censoring weights, zero entry,
inclusive entry ties, unsupported early horizons, and subset alignment.
It independently checks censoring cumulative hazard against
`survival::survfit` on untied counting-process data.

A discrete oracle example has true post-entry survival `2/3`; its expected
Brier score is `2/9`. The corrected score reproduces this and penalizes a
distorted probability. A separate smoke test exercises the forest censoring
model. The test fails on the old implementation.

A five-subject exact sample from independent discrete source entry and
failure times directly reproduces the original bug. At time 2 the old score
is 0.2481481 for the true curve and 0.2463959 for a distorted curve: it prefers
the wrong prediction. The revised scores are 0.2333333 and 0.2345034,
respectively, so the true prediction scores better.
