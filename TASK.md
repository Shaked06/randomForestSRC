# Project: Extend randomForestSRC to Left-Truncated Right-Censored (LTRC) data

## Goal
Fork `kogalur/randomForestSRC` and modify its C core so the Random Survival
Forest correctly handles **left truncation (delayed entry)**, not just right
censoring. The whole change reduces to one principle:

Everywhere the standard RSF uses the right-censored at-risk indicator

$$Y_i(t) = \mathbb{1}(\tilde T_i \ge t)$$

replace it with the delayed-entry at-risk indicator

$$Y_i^{LT}(t) = \mathbb{1}(L_i \le t \le \tilde T_i)$$

where $L_i$ is the entry/truncation time. **Event counts never change** — only
the definition of who is "at risk" changes.

## Observed data
Per subject $i$: $(L_i, \tilde T_i, \delta_i, X_i)$ where

- $L_i$ = entry / left-truncation time
- $\tilde T_i = \min(T_i, C_i)$ = observed follow-up time
- $\delta_i = \mathbb{1}(T_i \le C_i)$ = event indicator
- $X_i$ = covariate vector

Only subjects with $L_i \le \tilde T_i$ are observed.

---

## The two edit sites

### Site 1 — the split rule (CONFIRMED)
**File:** `src/splitCustom.c`, function `getCustomSplitStatisticSurvival`.

The at-risk counts are built in this loop:

```c
if (eventTime[k] <= time[i]) {      // implements 1(T~_i >= t_k)
  nodeParentAtRisk[k]++;
  if (membership[i] == LEFT) nodeLeftAtRisk[k]++;
  ...
}
```

followed by a reverse cumulative sum that turns per-time counts into "at risk up
to $\tilde T_i$". This must become $\mathbb{1}(L_i \le t_k \le \tilde T_i)$ — add
the lower-bound condition on entry time. The cumulative-sum trick won't carry
over directly, because at-risk is now an **interval** $[L_i, \tilde T_i]$, not a
tail. Event-count logic (`nodeParentEvent`, `nodeLeftEvent`) and the
numerator/denominator (`deltaNum`, `deltaDen`) stay untouched.

Delayed-entry log-rank statistic to implement (project outline §6):

$$U^{LT} = \sum_{t_k} \left( d_L(t_k) - d(t_k)\frac{Y_L^{LT}(t_k)}{Y^{LT}(t_k)} \right)$$

$$V^{LT} = \sum_{t_k} \frac{Y_L^{LT}(t_k)\, Y_R^{LT}(t_k)\, d(t_k)\,\{Y^{LT}(t_k)-d(t_k)\}}{\{Y^{LT}(t_k)\}^2 \{Y^{LT}(t_k)-1\}}$$

$$Z^{LT} = \frac{U^{LT}}{\sqrt{V^{LT}}}$$

Split maximizes $|Z^{LT}|$ over candidate variables and cutpoints. Terms with
$Y^{LT}(t_k) \le 1$ are omitted.

### Site 2 — the terminal-node estimator (TO CONFIRM)
**File:** likely `src/survival.c` or `src/survivalE.c` (confirm exact
file/function first).

Terminal-node Nelson–Aalen cumulative hazard (project outline §7):

$$\hat H_h^{LT}(t) = \sum_{t_k \le t} \frac{d_h(t_k)}{Y_h^{LT}(t_k)}, \qquad Y_h^{LT}(t_k) = \sum_{i\in h}\mathbb{1}(L_i \le t_k \le \tilde T_i)$$

Same at-risk substitution in the denominator; numerator $d_h(t_k)$ (event
counts) unchanged. **Not** reachable through the custom-split harness — must be
edited in core survival code. First task here is to locate the exact
file/function and its at-risk loop.

---

## The plumbing problem (shared first task)
The current C interfaces carry `time` and `event` but **no entry-time vector**.
Before either site can be modified, thread a new `truncationTime` ($L_i$) vector
from the R side → C entry point → both the split routine and the terminal-node
routine. This shared plumbing is the real first coding step.

## Path decision
The custom-split-rule route (`splitrule="custom"`, recompile, no core edits)
covers **only Site 1**. Site 2's terminal-node estimator is always the standard
right-censored Nelson–Aalen and can't be customized through that harness. A
faithful LTRC-RSF needs both, so we commit to **direct core modification** of
both sites plus the $L_i$ plumbing.

---

## Setup & build loop
```bash
git clone https://github.com/<you>/randomForestSRC.git
cd randomForestSRC
git remote add upstream https://github.com/kogalur/randomForestSRC.git
git checkout -b ltrc-rsf
R CMD INSTALL --preclean .     # establish a known-good baseline BEFORE any edits
```
Every C change requires a recompile (`R CMD INSTALL --preclean .` or, for faster
iteration, `devtools::load_all(".")`). Default install is serial, which is fine
for debugging.

## Suggested order of work
1. Establish a clean baseline build on the fork.
2. Pull `src/survival*.c`; confirm Site 2's exact location and at-risk loop.
3. Trace R → C response-data passing; design where to inject the `L_i` vector.
4. Implement the shared `truncationTime` plumbing.
5. Modify Site 1 (split at-risk → interval indicator + delayed-entry log-rank).
6. Modify Site 2 (terminal-node at-risk → interval indicator).
7. Later: OOB / prediction-error metrics that respect delayed entry
   (LT-aware C-index, Brier score).

## Validation plan
- Baseline sanity: with all $L_i = 0$, the LTRC forest must reproduce the
  standard right-censored RSF exactly (the interval $[0, \tilde T_i]$ collapses
  to the tail $\{\tilde T_i \ge t\}$).
- Simulation: compare (a) standard RSF ignoring truncation, (b) proposed LTRC
  RSF, (c) Cox PH with left truncation. Report LT-aware C-index, integrated
  Brier score with delayed-entry correction, and calibration of predicted
  survival in covariate strata.

## Reference
Full math in the project outline PDF (`left_truncated_RSF_student_project`):
delayed-entry log-rank (§6), terminal-node estimator (§7), OOB prediction (§8),
prediction-error evaluation (§9), full algorithm (§10). Base method: Ishwaran
et al. (2008), Random Survival Forests, *Ann. Appl. Stat.* 2(3), 841–860.
