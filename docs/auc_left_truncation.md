# Time-Dependent AUC with Left Truncation

The **time-dependent AUC** evaluates how well a model's predicted risk *ranks*
subjects at a specific time point $t$. It sits between the two metrics already
implemented here: like the $C$-index it is rank-based (it never asks whether a
predicted probability is numerically right), but like the Brier score it is
evaluated **at a fixed time $t$**, against the risk set alive at that time —
which is exactly where left truncation does its damage.

> **Naming note.** The existing exported `get.auc()` in this package is the
> Hand & Till (2001) *multiclass classification* AUC. It has nothing to do with
> survival, time, or truncation. The estimator below is a new function,
> `get.auc.survival()`, named to parallel `get.brier.survival()`.

---

## 1. Standard Time-Dependent AUC (Right-Censored Data)

We use the **cumulative/dynamic** (C/D) definition. At evaluation time $t$ the
sample splits into:

- **Cases** $\mathcal{D}(t) = \{i : T_i \le t,\ \delta_i = 1\}$ — subjects who
  have had the event by $t$.
- **Controls** $\mathcal{C}(t) = \{j : T_j > t\}$ — subjects still event-free
  at $t$.

Each subject gets a **marker** $M_i(t)$: the predicted risk of the event
occurring by $t$,

$$M_i(t) = 1 - \hat{S}(t \mid X_i)$$

The AUC is the probability that a randomly chosen case is ranked riskier than
a randomly chosen control:

$$\text{AUC}(t) = P\big(M_i(t) > M_j(t) \;\big|\; i \in \mathcal{D}(t),\, j \in \mathcal{C}(t)\big)$$

### Handling Right Censoring (IPCW)

Censoring before $t$ hides whether a subject is a case or a control, so each
subject is reweighted by the inverse of its probability of *not* being
censored, using the Kaplan-Meier censoring distribution
$\hat{G}_C(t) = P(C > t)$:

$$w_i = \frac{1}{\hat{G}_C(T_i^-)} \quad (\text{cases}), \qquad w_j = \frac{1}{\hat{G}_C(t)} \quad (\text{controls})$$

$$\widehat{\text{AUC}}(t) = \frac{\displaystyle\sum_{i \in \mathcal{D}(t)} \sum_{j \in \mathcal{C}(t)} w_i\, w_j \left[ \mathbb{I}(M_i > M_j) + \tfrac{1}{2}\mathbb{I}(M_i = M_j) \right]}{\displaystyle\left(\sum_{i \in \mathcal{D}(t)} w_i\right)\left(\sum_{j \in \mathcal{C}(t)} w_j\right)}$$

- **Interpretation:** $0.5$ is coin-flip ranking, $1.0$ is perfect ranking.
  Note this is **higher-is-better**, unlike MAD, $1-C$, and the Brier score.
- The $\tfrac{1}{2}\mathbb{I}(M_i = M_j)$ term is not cosmetic here: forest
  subjects landing in the same terminal nodes across all trees receive
  *identical* survival curves, so exact ties are common.

---

## 2. Adapting the Time-Dependent AUC to Left Truncation

The same three adjustments as the Brier score apply, for the same reasons —
see `brier_score_left_truncation.md`.

### Adjustment 1: Dynamic Risk Set at Time $t$

A subject entering at $T_{L,i} > t$ was not under observation at $t$ and
cannot serve as a case or a control there:

$$\mathcal{R}_L(t) = \{i : T_{L,i} \le t\}, \qquad
\mathcal{D}_L(t) = \mathcal{D}(t) \cap \mathcal{R}_L(t), \qquad
\mathcal{C}_L(t) = \mathcal{C}(t) \cap \mathcal{R}_L(t)$$

> **Only controls are ever actually dropped.** For a case we always have
> $T_{L,i} \le T_i \le t$, so $\mathcal{D}_L(t) = \mathcal{D}(t)$ identically.
> The restriction bites only on the control side, removing subjects with
> $T_j > t$ who had not yet entered at $t$. This is worth stating because it
> pins down the direction of the bias: an unadjusted AUC pads the control
> pool with subjects who were *guaranteed* to be event-free at $t$ by
> construction, not by prognosis.

### Adjustment 2: Conditional Survival Predictions

A subject under observation has already survived to its own entry time, so
the relevant predicted risk at $t$ is conditional on that:

$$M_i(t) = 1 - \frac{\hat{S}(t \mid X_i)}{\hat{S}(T_{L,i} \mid X_i)}$$

This genuinely reorders subjects rather than rescaling them all the same way:
the divisor $\hat{S}(T_{L,i} \mid X_i)$ is specific to subject $i$, so two
subjects with the same $\hat{S}(t \mid X_i)$ but different entry times get
different markers. A rank-based metric therefore does *not* get to ignore
this adjustment.

### Adjustment 3: Truncation Weights

Alongside the censoring weight, each subject is weighted by the inverse
probability of having been *observed at all* — i.e. of entering early enough
to appear in the sample. With $G_L(t) = P(T_L \le t)$:

$$w_i = \frac{1}{\hat{G}_C(T_i^-)\, \hat{G}_L(T_i^-)} \quad (\text{cases}),
\qquad
w_j = \frac{1}{\hat{G}_C(t)\, \hat{G}_L(t)} \quad (\text{controls})$$

$G_L$ is estimated by the plain empirical CDF of the observed entry times.
Unlike the censoring distribution, entry times are directly observed for
every retained subject, so no risk-set/KM machinery is needed.

---

## 3. Left-Truncated Time-Dependent AUC Formula ($\text{AUC}_{\text{LT}}$)

$$\widehat{\text{AUC}}_{\text{LT}}(t) = \frac{\displaystyle\sum_{i \in \mathcal{D}_L(t)} \sum_{j \in \mathcal{C}_L(t)} w_i\, w_j \left[ \mathbb{I}(M_i > M_j) + \tfrac{1}{2}\mathbb{I}(M_i = M_j) \right]}{\displaystyle\left(\sum_{i \in \mathcal{D}_L(t)} w_i\right)\left(\sum_{j \in \mathcal{C}_L(t)} w_j\right)}$$

with $M_i$ the conditional marker of Adjustment 2.

### A useful cancellation (and why the code does not rely on it)

Under `cens.model = "km"` the control weight
$1/(\hat{G}_C(t)\hat{G}_L(t))$ is the *same constant for every control* at a
given $t$, so it cancels exactly between numerator and denominator. Only the
case weights actually matter there. The implementation still carries control
weights explicitly, because under `cens.model = "rfsrc"` the censoring
distribution is subject-specific and the cancellation no longer holds.

### Undefined points

$\widehat{\text{AUC}}_{\text{LT}}(t)$ is `NA` when either
$\mathcal{D}_L(t)$ or $\mathcal{C}_L(t)$ is empty — routine at the very start
and very end of the grid — or when the weights are not finite (a zero
estimated $G_C$ or $G_L$ in the denominator).

### Summary statistic

The headline number is the **time-averaged AUC**: the trapezoidal integral of
$\widehat{\text{AUC}}_{\text{LT}}(t)$ over the grid points where it is
defined, divided by the width of that range. This mirrors how
`get.brier.survival()` reports `crps.std`, and keeps the summary figure and
the plotted curve from ever disagreeing.

---

## 4. Summary

The time-dependent AUC for left truncation evaluates ranking quality at a
fixed time $t$ by:

1. Restricting both the case and the control pool to subjects who had entered
   by $t$ ($T_{L,i} \le t$) — which in practice only ever removes controls.
2. Conditioning each predicted risk on survival up to that subject's own
   delayed entry $T_{L,i}$.
3. Weighting observations via joint IPCW / inverse probability of truncation
   weighting, at $T_i^-$ for cases and at $t$ for controls.
