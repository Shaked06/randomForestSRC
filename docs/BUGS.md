# Bug tracker

This is the live list for the left-truncation work. IDs match the original
adversarial review. Each fix gets regression tests and its own local commit
after review and approval. A local commit does not imply a GitHub push.

| ID | Priority | Bug | Origin | Status | Fix commit |
|---|---|---|---|---|---|
| 1 | P1 | Saved-model prediction crashes or uses entry times from another fit | LT integration | Fixed locally | `4127a1a` |
| 2 | P1 | Missing follow-up imputation can place exit before entry and return negative survival | LT integration | Fixed locally | `d057c9e` |
| 3 | P1 | Restored training error and standalone VIMP ignore entry | LT integration | Fixed locally | `a4c890e` |
| 4 | P1 | Cox time-grid indexing corrupts simulation predictions | Our simulations | Fixed locally; results rerun pending | This commit |
| 5 | P1 | LT Brier construction can favor incorrect survival probabilities | LT metric design | Open | — |
| 6 | P1 | Brier/AUC weights mistake observed entry CDF for source entry CDF | LT metric design | Open | — |
| 7 | P1 | Prediction-object scoring uses training outcomes | Inherited Brier bug; propagated to our AUC | Open | — |
| 8 | P2 | Brier subsetting misaligns curves and outcomes | Inherited Brier bug; LT indexing repeats it | Open | — |
| 9 | P2 | Accepted entry/event ties cause infinite weights | LT metrics | Open | — |
| 10 | P2 | NA omission does not subset entry times | LT integration | Open | — |
| 11 | P2 | Tuning holdouts do not subset entry times | LT integration | Open | — |
| 12 | P2 | Ordinary forests lose fast concordance scoring | LT integration | Open | — |
| 13 | P2 | Unsupported splitting rules silently accept entry times | LT integration | Open | — |
| 14 | P2 | Package fixes do not invalidate simulation caches | Our simulations | Open | — |
| 15 | P2 | Full-ensemble partial dependence uses an OOB normalization array | Pre-LT source; exposed during fix 1 | Needs dedicated reproduction/test | — |

P1: crash, incorrect results, or invalid statistical interpretation.
P2: feature failure, compatibility gap, performance, or reproducibility issue.
“Inherited” refers to our starting repository, not a verified current CRAN release.

## Fix records

### 1 — Forest-owned entry times

- Commit: `4127a1a`.
- Tests: `tests/entry-time-prediction-state.R` and
  `tests/entry-time-prediction-serialization.R`.
- Tests fail on the pre-fix build and pass on the fixed build.
- Covers ordinary/LT fits, both terminal-storage modes, restored prediction,
  direct forest prediction, and default OOB partial dependence in fresh R sessions.
- Older saved LT objects without any stored entry times need refitting.

### 2 — Missing follow-up with positive entry

- Commit: `d057c9e`, `Reject missing follow-up times with delayed entry`.
- Behavior: reject missing follow-up times for subjects with positive
  entry before calling the native algorithm. This is a safety restriction,
  not a new delayed-entry imputation estimator.
- Missing predictors and zero-entry response imputation remain supported.
- The same validation protects restoration of legacy forests, including
  stored follow-up times that are already below entry.
- Regression test: `tests/entry-time-missing-followup.R`. It fails on the
  previous build with survival -9 and passes with the guard.
- Covers one and multiple imputation iterations, NA/NaN, missing predictors,
  missing status, zero-entry equivalence, mixed entry, and legacy restoration.
- Verification: fresh package build and `R CMD check` with examples/manual/
  vignettes skipped; all three regression test files pass, status OK.
- Approved for a local commit. No GitHub push authorized.

### 3 — Entry-aware restored performance and VIMP

- Commit: `a4c890e`, `Respect entry times in restored survival performance`.
- Change: supply training entry times to concordance in both grow
  and restore modes. New-data prediction must not use the training vector.
- Scope: an omitted entry vector and an explicit zero vector give identical
  results. Positive entries before all events can also be inert. The defect
  matters when entry times remove otherwise comparable pairs; non-NULL alone
  is not a sufficient trigger.
- Before fix: a fixture has identical OOB predictions but grow error
  0.5990415 versus restored error 0.6052801. With the fix both are
  0.5990415.
- Independent one-split-tree reference: anti-VIMP is 0.1974790; restoration
  previously returned 0.2380165. Both individual and joint VIMP now match the
  entry-aware reference.
- Tests: `tests/entry-time-restored-performance.R` and
  `tests/entry-time-restored-vimp.R`. Both fail on the pre-fix build and pass
  with the change.
- Covers both concordance weighting settings, both terminal-storage modes,
  direct forest restoration, new-data and pseudo-restoration scoring, and
  the reference importance/error used by subsampling and double bootstrap.
  Interval-reference checks use the default Uno setting; they do not validate
  every resampling option or the coverage of confidence intervals.
- Verification: fresh source build and package checks with examples/manual/
  vignettes skipped; all five regression test files pass, status OK.
- Approved for a local commit. No GitHub push authorized.

### 4 — Cox survival predictions on a shared grid

- Commit: this commit, `Fix Cox survival predictions before the first follow-up time`.
- Change: prepend a zero baseline hazard and use `idx + 1` to
  preserve every grid position, including times before observed follow-up.
  R drops zero indices in the original lookup; `ifelse` then recycles and
  misaligns the shorter hazard vector.
- Before fix: a simple curve rises from survival 0.2960 at time 9 to 0.8825
  at time 12. The corrected curve remains at 0.2960.
- Regression test: `simulations/tests/cox_survival_matrix.R`. It fails on
  the pre-fix helper and passes with the change. Run it from the
  repository root with `Rscript simulations/tests/cox_survival_matrix.R`.
- Independent reference: `survival::survfit`, using a literal per-time lookup.
  Covers 4 Cox fits (including delayed entry, ties, and early censoring),
  7 grid cases, and both one and multiple test subjects. Checks values,
  dimensions, probability bounds, and monotonicity.
- Saved-run audit reconstructs both Cox phases for the existing ten draws
  in all six scenarios. The old helper reproduces the stored MAD values;
  all corrected curves agree with `survfit` within 1.6e-15. Scenarios 1, 2,
  4, and 6 change; 3 and 5 do not.
- Example: scenario 1 phase-2 Cox MAD changes from 0.02783970 to 0.01021246.
  Full diagnostic values: `docs/review/evidence/fix4-cox-grid-impact.csv`.
- An end-to-end scenario-1 smoke replication passes. Standard versus
  zero-entry gaps are 2.33e-15 for Cox and exactly zero for RSF.
- Existing saved results and rendered reports remain historical. Regeneration
  is pending; metric corrections in bugs 5–6 are also still outstanding.
- No R package source changed in this fix; simulation tests run separately
  from the package regression tests. Approved for a local commit; no GitHub
  push authorized.
