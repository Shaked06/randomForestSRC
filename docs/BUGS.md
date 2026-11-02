# Bug tracker

This is the live list for the left-truncation work. IDs match the original
adversarial review. Each fix gets regression tests and its own local commit
after review and approval. A local commit does not imply a GitHub push.

| ID | Priority | Bug | Origin | Status | Fix commit |
|---|---|---|---|---|---|
| 1 | P1 | Saved-model prediction crashes or uses entry times from another fit | LT integration | Fixed locally | `4127a1a` |
| 2 | P1 | Missing follow-up imputation can place exit before entry and return negative survival | LT integration | Fixed locally | `d057c9e` |
| 3 | P1 | Restored training error and standalone VIMP ignore entry | LT integration | Fixed locally | `a4c890e` |
| 4 | P1 | Cox time-grid indexing corrupts simulation predictions | Our simulations | Fixed locally; results rerun pending | `820b285` |
| 5 | P1 | LT Brier construction can favor incorrect survival probabilities | LT metric design | Fixed locally | `539df42` |
| 6 | P1 | Brier/AUC weights mistake observed entry CDF for source entry CDF | LT metric design | Fixed locally | This commit |
| 7 | P1 | Prediction-object scoring uses training outcomes | Inherited Brier bug; propagated to our AUC | Open | — |
| 8 | P2 | Brier subsetting misaligns curves and outcomes | Inherited Brier bug; LT indexing repeats it | LT path covered in fix 5; ordinary Brier open | — |
| 9 | P2 | Accepted entry/event ties cause infinite weights | LT metrics | Fixed locally by fixes 5 and 6 | `539df42` + this commit |
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

- Commit: `820b285`, `Fix Cox survival predictions before the first follow-up time`.
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

### 5 — Entry-restricted Brier score

- Commit: `539df42`, `Correct entry-conditioned Brier scoring`.
- Governing reference: professor's project outline, section 9.2, page 7,
  in `docs/thesis/left_truncated_RSF_student_project.pdf`.
- Interpretation: score conditional prediction after entry for enrolled
  subjects. Condition both outcome predictions and censoring probabilities
  on reaching entry; estimate censoring with delayed-entry risk sets.
  The expanded formula is our derivation, not a verbatim formula in the PDF.
- Remove additional inverse entry-CDF weights. A five-row exact truncated
  sample previously scores the true curve 0.2481481 and a distorted curve
  0.2463959 (lower is better). The corrected scores are 0.2333333 and
  0.2345034, restoring the correct ordering.
- Preserve the historical exponential Nelson–Aalen censoring estimator
  under the `km` option, changing its risk sets. The pooled model assumes
  a common censoring hazard. The `rfsrc` option uses covariates and entry,
  fits with delayed entry, and preserves rows with missing predictors.
- Regression test: `tests/entry-time-brier.R`; fails before the fix and
  passes on the rebuilt package. Includes exact oracle losses, hand
  censoring calculations, an independent `survfit` cumulative-hazard
  reference, off-grid weights, inclusive ties, subset/one-row alignment,
  missing OOB predictions, undefined horizons, and forest censoring.
- Verification: isolated build and `R CMD check` with examples/manual/
  vignettes skipped, status OK; all six regression files pass. Expanded
  final Brier cases also pass against the installed isolated build.
- Compatibility audit: ordinary NULL-entry Brier and NULL/LT AUC results
  are identical to the preceding code across four subject selections.
  Explicit zero entry has the ordinary IPCW interpretation, but actual-time
  lookups and left limits can differ from legacy coarse-grid/tie handling.
- Overlap: Brier no longer needs bug 6's entry CDF; its LT path handles
  bugs 8 and 9. AUC's CDF/tie bugs and ordinary Brier subsetting remain open.
  Bug 7 (prediction-object outcomes) is not addressed.
- Formula, assumptions, and limits: `docs/brier_score_left_truncation.md`.
  Scientific review with the professor remains appropriate before treating
  this interpretation as settled in the thesis. No GitHub push authorized.

### 6 — Remove source-entry weights from entered-cohort AUC

- Approved for a local commit, `Correct entry-conditioned survival AUC`.
  Brier no longer uses the erroneous CDF after fix 5; this correction
  addresses AUC.
- Cause: the empirical entry CDF among retained subjects is generally not
  the source entry CDF. For independent uniform source entry and failure
  times, the two CDFs at .5 are .75 and .5, respectively.
- Target: cumulative/dynamic ranking among enrolled subjects after entry,
  consistent with the Brier interpretation approved in fix 5. This is our
  project extension: the professor's outline does not specify AUC.
  It does not estimate source-population AUC from time zero.
- Change: remove the inverse entry-CDF weights. Use conditional markers
  based on `S(t)/S(L^-)` and observation weights `G(L^-)/G(U^-)` for cases,
  `G(L^-)/G(t)` for controls. Share the already tested Brier conditioning
  and delayed-entry censoring helpers, preserving its results.
- Exact counterexample: five equally likely retained combinations from
  source `T` uniform on `{1,2,3}` and `L` uniform on `{0,1.5}`. Direct
  pairwise AUC at time 2 is 7/12 = 0.5833333; the old entry weights return
  27/44 = 0.6136364. The corrected scorer matches the direct result.
- Inclusive entry ties retain their cases, also resolving AUC's part of
  bug 9. Unsupported required censoring probabilities raise an error;
  absent cases or controls produce NA. Missing OOB predictions and subjects
  who have not entered are excluded from pair comparisons.
- Regression test: `tests/entry-time-auc.R`, fails before and passes after.
  Includes hand-computed delayed-entry weights, event/censor ties, marker
  ties, subsets, early horizons, one-point grids, missing OOB predictions,
  independent `survfit` cumulative hazards, explicit quadratic weighted-pair
  references on eight random fixtures, and an actual delayed-entry forest.
  Forest-censoring curves also match Brier under the same seed with missing
  predictors present.
- Verification: isolated source build and `R CMD check` with examples/manual/
  vignettes skipped, status OK; all seven regression files pass. The expanded
  final AUC cases also pass on the installed isolated build.
- Compatibility audit: NULL/LT Brier results and NULL-entry AUC results
  are identical to fix 5 across two censoring models and four selections.
- Details, modeling assumptions, and primary-source context:
  `docs/auc_left_truncation.md`. The historical descriptive `iauc` average
  is retained, including its possible bridging of undefined interior points.
- Bug 7 (new-data prediction-object outcomes) remains open. Saved simulation
  outputs are not regenerated. No GitHub push authorized.
