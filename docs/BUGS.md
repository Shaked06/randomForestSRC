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
| 6 | P1 | Brier/AUC weights mistake observed entry CDF for source entry CDF | LT metric design | Fixed locally | `42d7a66` |
| 7 | P1 | Prediction-object scoring uses training outcomes | Inherited from pre-LT snapshot; propagated to our AUC | Fixed locally by upstream integration | `cb60a6e`; upstream `30d95e2` + LT adaptation |
| 8 | P2 | Brier subsetting misaligns curves and outcomes | Inherited Brier bug; LT indexing repeats it | Fixed locally by upstream integration | `cb60a6e`; LT fix `539df42` |
| 9 | P2 | Accepted entry/event ties cause infinite weights | LT metrics | Fixed locally by fixes 5 and 6 | `539df42` + `42d7a66` |
| 10 | P2 | NA omission does not subset entry times | LT integration | Fixed locally | `a62897b` |
| 11 | P2 | Tuning holdouts do not subset entry times | LT integration | Fixed locally | `ef64356` |
| 12 | P2 | Ordinary forests lose fast concordance scoring | LT integration | Fixed locally | `107369e` |
| 13 | P2 | Unsupported splitting rules silently accept entry times | LT integration | Fixed locally | `87887c3` |
| 14 | P2 | Package fixes do not invalidate Stage 1 simulation caches | Our simulations | Fixed locally | This commit |
| 15 | P2 | Full-ensemble partial dependence uses an OOB normalization array | Pre-LT source; exposed during fix 1 | Needs dedicated reproduction/test | — |

P1: crash, incorrect results, or invalid statistical interpretation.
P2: feature failure, compatibility gap, performance, or reproducibility issue.
“Inherited” refers to our starting repository, not a verified current CRAN release.

## Performance follow-up

- Develop and validate a fast concordance algorithm for positive entry
  times. Current LT scoring adapts the existing pairwise algorithm;
  bug 12 restores the existing fast algorithm only for NULL/all-zero entry.
  This is a separate enhancement, including weighted/unweighted scores,
  entry/event ties, and independent correctness/performance comparisons.

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

- Commit: `42d7a66`, `Correct entry-conditioned survival AUC`.
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

### 7 — Provenance and tuning scope, verified before implementation

- Resolved by this approved merge: upstream separation is adapted to LT
  scoring, with independent evaluation-outcome regression tests.
- Inherited Brier defect reproduced using unchanged source from pre-LT
  snapshot `145ac2f8692a724fcd451960dd4ef4fbe74ff08a` (2026-04-19).
  Our shared setup propagated it to the added survival AUC helper.
- Current `kogalur/randomForestSRC` main, checked on 2026-11-02 at
  `30d95e206b8e287863afb55970014497e3803a8f` (commit dated 2026-10-27),
  separates grow and evaluation outcomes. Its unchanged R scorer matches
  independent Brier losses on real prediction objects and rejects absent
  evaluation outcomes. Do not describe this defect as present in current
  upstream or infer current CRAN behavior from the older snapshot.
- Built-in `tune()` and `tune.nodesize()` use `get.mv.error()` and native
  prediction errors rather than this Brier helper. Independent no-censoring
  pair counting matches those native errors on the provenance fixture.
  Custom tuning that calls the affected local Brier/AUC helpers on new-data
  prediction objects can still be affected.
- Full evidence, limitations, and reproduction:
  `docs/review/BUG7_PROVENANCE.md`.

## Approved upstream integration — 2026-11-06

The user approved this local merge after reviewing the tested integration
on `integrate-upstream-2026-11-06`. It joins LT parent `42d7a66` and upstream
parent `30d95e2`, preserving both histories. See
`docs/review/UPSTREAM_SYNC_REVIEW.md` for the exact parents, changed behavior,
tests, and review artifacts. No GitHub push is authorized.

- Bugs 1–6 and 9 retain their committed fixes. All existing LT tests pass
  after reconciliation with upstream's native RNG interface.
- Bug 7: upstream's grow/evaluation separation now also drives LT scoring.
  Different-size and equal-size validation sets match independent losses;
  absent validation outcomes raise a clear error. The censoring distribution
  uses training outcomes and stored training entry times, while evaluation
  uses validation outcomes/entries. This is covered for both AUC names.
- Bug 8: ordinary Brier subset permutations and selected validation rows
  pass; the upstream subset correction is retained alongside the LT path.
- The `km` censoring option now follows upstream's product-limit estimator,
  with delayed-entry risk sets for LT and failures before censoring at ties.
  This supersedes the exponential Nelson–Aalen choice recorded in fixes 5–6.
  Uncensored oracle examples retain their expected values; censored values
  and newly grown forests can change with the estimator/RNG updates.
- Bugs 10 and 11 still reject un-subsetted entry vectors after NA omission
  or tuning holdouts. Both were reproduced against the integrated build.
- Bug 12: the native code still disables Fenwick scoring for non-NULL entry,
  including an all-zero vector. Numerical zero/NULL equality was checked;
  this is not a new timing benchmark or a performance fix.
- Bug 13: both `logrankscore` and `bs.gradient` still accept positive entry
  without warning. Their LT validity has not been established.
- Bug 14: the stale simulation-cache reproduction still passes. An upstream
  version bump does not supply the missing implementation fingerprint for
  subsequent edits. Full saved simulations remain historical.
- Bug 15: full-ensemble partial-dependence normalization still needs its
  dedicated reproduction/test. Default OOB partial regression passes; this
  does not validate `oob = FALSE`.

Verification: isolated package build/check, status OK (examples/manual/
vignettes skipped), all eight regression files pass; all six reduced-size
simulation smoke cases pass; 12 ordinary profiles match pristine upstream
and explicit-zero integration across two RNG protocols and three families;
four saved pre-integration forests preserve restored/new-data predictions.

### 10 — Align grow-time entry values with retained observations

- Commit: `a62897b`, `Align entry times with retained training rows`.
- Origin: LT integration. Ordinary row omission already worked, but the
  external entry vector was not updated when the data lost rows.
- Before: a valid original-row entry vector is rejected after NA processing
  reduces the fitted sample. The new regression's first fixture supplies
  80 rows, omits 4, and fails on the previous build with the entry-length
  error instead of fitting the 76 retained subjects.
- Change: capture original input row identities before preprocessing and
  align entries to the exact rows surviving both preliminary cleanup and
  final NA omission. Preserve retained order and store that vector on the
  grow object and its saved forest. Entry vector names do not override
  positional alignment with input data.
- This does not add row deletion or change the missing-data policy.
  Entirely empty rows already removed during imputation preprocessing also
  lose their entries; partially missing predictor rows remain imputable.
  NULL retains ordinary zero-entry behavior without storing an entry vector.
- Supply one entry per original input row, before NA processing. A shorter
  pre-filtered vector paired with unfiltered data is rejected explicitly.
  Retained values are checked for numeric type, finite/non-negative values,
  missingness, and entry not exceeding follow-up. Omitted subjects' entry
  values are discarded with them before retained-value validation.
- Regression: `tests/entry-time-na-omission.R`; fails before and passes
  against the rebuilt package. It compares grow predictions, OOB curves,
  error rates, data alignment, saved entries, and restoration with manually
  cleaned data and matching entries under the same seed.
- Covers missing predictors/time/status, NA/NaN, first/middle/final rows,
  entirely empty rows, both memory settings, matrix input, custom row names,
  named entry vectors, factor-level cleanup, unused missing columns,
  predictor imputation, explicit zeros/NULL, fast-wrapper delegation, and
  invalid lengths/retained values. Custom bootstrap/weight alignment is not
  changed by this fix.
- Verification: isolated source build and `R CMD check`, status OK, with
  examples/manual/vignettes skipped; all nine package regression files pass.
- Bug 11's tuning-holdout subsetting remains a separate open task.
  No GitHub push authorized.

### 11 — Match tuning entries to training and holdout subjects

- Commit: `ef64356`, `Preserve entry times and LT scoring during tuning`.
- Origin: LT integration in both `tune()` and `tune.nodesize()`. Before the
  fix, reserving a holdout passed the full entry vector to the smaller
  training frame. Joint tuning fails length validation; nodesize tuning
  can return no valid errors. Preprocessing could also shorten the frame
  before entries were aligned.
- Change: pass entries through the preprocessing stump so fix 10 aligns
  them with the cleaned frame. Candidate fits receive training entries;
  the optional final full-data fit receives all cleaned entries. Both grid
  and golden searches use the same aligned split.
- Correct LT holdout loss as well as the length error: use the held-out
  subjects' own entry times to restrict comparable concordance pairs.
  Reuse the existing prediction-derived Uno weights, which come from the
  training censoring fit. `use.uno = FALSE` retains unweighted concordance.
  No new concordance weighting estimator or native prediction-entry API is
  introduced. NULL/zero-entry holdout scoring retains native errors;
  OOB scoring continues to use the grow-time entry-aware error.
- Regression: `tests/entry-time-tuning.R`, fails before and passes on the
  rebuilt package. It inspects real grow calls for entry/subject alignment
  and independently enumerates held-out pairs using the supplied weights.
  Fixtures require a difference from entry-blind error, so merely avoiding
  the length failure is insufficient to pass.
- Covers both tuners, grid/golden searches, weighted/unweighted holdouts,
  OOB fallback, missing-data preprocessing, original-row input validation,
  final full-data entry storage, explicit-zero/NULL equivalence, and caller
  RNG-state preservation. The independent reference checks use continuous
  follow-up times; existing concordance code handles event-time ties.
- Compatibility audit: 24 ordinary profiles match the pre-fix functions
  across both tuners/searches, holdout/OOB, and survival/regression/
  classification. Previously working complete-data LT OOB behavior is
  checked separately. Custom bootstrap/weight alignment and the tuners'
  existing preprocessing policy are outside this fix.
- Verification: isolated build and `R CMD check`, status OK, with
  examples/manual/vignettes skipped; all ten regression files pass.
- No GitHub push authorized.

### 12 — Restore fast concordance for ordinary survival fits

- Commit: `107369e`, `Restore fast concordance for zero entry times`.
- Origin: LT integration. Native grow/restore calls supply an all-zero
  entry vector even for ordinary survival data. The LT guard treated any
  non-NULL vector as requiring the pairwise algorithm, disabling the
  existing fast algorithm for those ordinary fits.
- Change: permit the existing automatic/explicit fast selection for NULL
  or all-zero entry vectors. Any nonzero entry still forces the pairwise,
  entry-aware calculation, even with `fast = TRUE`. Positive entries before
  every failure also retain that conservative path. An explicit request for
  the slow algorithm is respected. This restores ordinary scoring speed;
  it does not implement a fast algorithm for positive LT entries.
- Optional concordance tracing reports the chosen algorithm. Console
  output is suppressed inside active OpenMP parallel regions.
- Regression: `tests/entry-time-fast-concordance.R` checks actual algorithm
  selection, score agreement, independent pair enumeration, event/risk
  ties, missing/zero weights, both automatic thresholds, explicit fast/slow
  choices, real grow/restore calls, and weighted/unweighted LT restrictions.
  The zero-entry selection assertion fails against pre-fix dispatch with
  only the same diagnostic added; all cases pass after the fix.
- Verification: isolated source build and `R CMD check`, status OK, with
  examples/manual/vignettes skipped; all eleven regression files pass.
  The local build is serial; an OpenMP runtime build was not tested.
- Timing evidence: `docs/review/evidence/fix12-concordance-timing.csv`,
  reproduced by `docs/review/repros/fix12-concordance-timing.R` against
  isolated before/after libraries. Each call scores 20,000 subjects; timings
  average three calls. Zero-entry scoring drops from roughly one second
  to about three milliseconds locally, matching the NULL-entry path.
  These are illustrative single-machine timings, not test thresholds or
  whole-forest speedup estimates.
- Numerical scope: the existing fast/pairwise algorithms can differ
  slightly through floating-point accumulation, and their epsilon-based
  grouping of nearly equal values is not universally identical. The timing
  fixture uses integer times/risks, checks score agreement within `1e-8`,
  and records the actual scores; its weighted baseline difference is about
  `1.5e-11`. Regression fixtures check their reference scores within `1e-12`.
  This fix restores the upstream algorithm choice without redesigning
  either concordance estimator.
- No GitHub push authorized.

### 13 — Reject splitting rules that have not been adapted to entry times

- Commit: `87887c3`, `Reject unsupported splitting rules with delayed entry`.
- Origin: LT integration omitted a guard around the package's existing
  split-rule choices. On pre-fix build `107369e`, `logrankscore`,
  `bs.gradient`, `custom`, and `custom1` all accept positive entries without
  warning. In the reproduction, logrankscore's stored native partition
  arrays and leaf counts are identical to a zero-entry fit under the same
  seed, while survival estimates change.
- Source findings: logrankscore's ranks use exit times and status without
  entry times (`src/splitSurv.c`). Brier-gradient splitting calls the shared
  entry-aware risk-count helper, but its gamma/subject-weight calculation
  still includes subjects before entry (`src/splitUtilSurv.c`). It is
  partially adapted, not a completely entry-blind calculation. Custom
  survival callbacks are not supplied an entry vector
  (`src/splitCustomDriver.c`). These rules do not implement our LT splitting
  procedure; an entry-aware leaf estimator does not adapt their split score.
- Change: after original-row entry alignment, NA processing, and split-rule
  resolution, allow positive retained entries only with single-event
  `logrank` or `random`. The former has the implemented LT split statistic;
  the latter uses no outcome-based split statistic. Unsupported choices
  stop with an error naming the requested rule and supported alternatives.
  Even one positive entry before every failure triggers this conservative
  boundary. NULL/all-zero entries retain ordinary rule selection.
- Rule abbreviations are resolved first. Multiple event types remap
  `logrank` to a different native statistic, so that request is rejected
  with positive entries too. Allowing random splitting does not establish
  general LT validity for competing-risk analyses; that extension remains
  outside this fix. No split statistic or saved forest is rewritten, and
  prediction/restoration of previously saved unsupported fits is unchanged.
- Regression: `tests/entry-time-splitrules.R` fails before and passes after
  the fix. Covers both alternative built-in rules, custom/custom1,
  mixed/early positive entries, resolved abbreviations, performance/output
  disabled, zero-depth requests, the fast wrapper, and multiple-event rule
  remapping. Checks zero/NULL equivalence of predictions, errors, and stored
  partitions, with Uno weighting enabled/disabled; verifies supported LT
  fits/restoration under both memory settings. Entries on omitted rows do
  not restrict the retained fit, while positive entries on retained or
  imputed rows still trigger the guard.
- Verification: isolated source build and `R CMD check`, status OK, with
  examples/manual/vignettes skipped; all twelve regression files pass.
  Documentation and NEWS describe the support boundary.
- No GitHub push authorized.

### 14 — Invalidate Stage 1 caches after a package implementation change

- Approved for a local commit, `Invalidate simulation caches after package implementation changes`.
- Origin: our simulation caching design. The replication-loop keys hashed
  setup/DGP/engine files and replication/test counts but omitted the
  installed package implementation. A same-version reinstall could leave
  cached predictions and scores from the previous implementation on screen.
- Change: fingerprint the installed `randomForestSRC` and `survival`
  DESCRIPTION/NAMESPACE, R files (including lazy-load databases), and native
  libraries by content. Include installation paths, R version, and platform.
  All six Stage 1 documents compute the provenance in their uncached library
  chunk and include it in the loop key alongside the existing inputs.
  This represents the installed code, not an uninstalled source checkout.
- New experiment results retain `package_provenance`, including their
  saved `results_phases.rds` representation. Workers receive the resolved
  installation directories and verify matching fingerprints before fitting.
  A startup mismatch closes the worker cluster and fails; a changed package
  between key construction and experiment startup also fails explicitly.
- Regression: `simulations/tests/package_cache.R` uses the actual loop
  header shared by all six documents and a temporary R/C package. Fresh R
  processes demonstrate real cache reuse with unchanged code, recomputation
  after an R-only reinstall, and recomputation after a native-only reinstall.
  Package version/metadata stay fixed; native artifacts are held fixed for
  the R-only case and R artifacts for the native-only case. The test fails
  before this fix because the direct value is .999 while cache returns .111.
  Setup, DGP, and engine changes still invalidate the cache independently.
- Worker regression: `simulations/tests/package_provenance.R` checks the
  chosen installation even without inherited library overrides, mismatch
  rejection/cluster cleanup, fingerprint stability and RNG preservation,
  and saved/reloaded provenance. A two-replication, 120-subject real Cox/RSF
  experiment matches serial replication scores, including independent
  zero-entry checks. All work uses temporary outputs; no saved run changes.
- Verification: both new regression scripts pass, all six Stage 1 documents'
  code chunks parse, and the existing Cox-grid regression passes all 56
  cases. Only simulation code/docs changed; the R package was not rebuilt
  for this fix. The prior package check remains the validation for the
  unchanged package implementation.
- Scope: protects Stage 1 replication caches. `setup.rds` remains an explicit
  hashed input; Stage 0 calibration/tuning caches are not changed. Existing
  saved runs remain historical and are not regenerated or relabeled.
  Reinstall edited package source and restart R before knitting. Fingerprints
  do not cover arbitrary in-memory namespace changes, every dependency,
  external system library, or compiler environment; this is not a complete
  environment lockfile. Paths/metadata may conservatively trigger extra runs.
- No GitHub push authorized.
