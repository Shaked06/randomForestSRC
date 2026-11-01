# Bug tracker

This is the live list for the left-truncation work. IDs match the original
adversarial review. Each fix gets regression tests and its own local commit
after review and approval. A local commit does not imply a GitHub push.

| ID | Priority | Bug | Origin | Status | Fix commit |
|---|---|---|---|---|---|
| 1 | P1 | Saved-model prediction crashes or uses entry times from another fit | LT integration | Fixed locally | `4127a1a` |
| 2 | P1 | Missing follow-up imputation can place exit before entry and return negative survival | LT integration | Fixed locally | This commit |
| 3 | P1 | Restored training error and standalone VIMP ignore entry | LT integration | Open | — |
| 4 | P1 | Cox time-grid indexing corrupts simulation predictions | Our simulations | Open | — |
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

- Commit: this commit, `Reject missing follow-up times with delayed entry`.
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
