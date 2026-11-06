> Historical review of the pre-fix LT branch. Findings describe that snapshot,
> not current upstream or the prepared integration. See `../BUGS.md` for live
> statuses, `BUG7_PROVENANCE.md` for attribution, and `UPSTREAM_SYNC_REVIEW.md`
> for the tested upstream integration.

# Adversarial review of the left-truncation extension

Reviewed the current working tree based on commit `f99b254`, including the existing local edits. Four adversarial reviewers covered the R interface, native implementation, evaluation metrics, and an independent challenge pass; the primary reviewer examined the simulation engine and reproduced result changes.

**Result: 14 actionable findings.** They include new integration defects, inherited bugs that affect this project, and one previously documented limitation now reproduced. This is a review, not a patch: no production R/C files, saved simulation results, or existing thesis files were changed.

P1 means a crash, silent incorrect results, or invalid statistical interpretation that should be resolved before relying on the affected functionality. P2 means a reproducible feature failure, missing compatibility guard, performance regression, or reproducibility defect.

## Immediate implications for the thesis

- The Cox numerical results in the existing reports and initial thesis table need correction: the survival-curve helper corrupts predictions on some time grids (finding 4).
- Brier and AUC implementations must not yet be described as validated LT estimators (findings 5–6). The Brier construction can reward a wrong probability model in a setting with no censoring.
- Training VIMP and confidence intervals are not uniformly entry-aware: restoration bypasses the correction (finding 3).
- “Held-out scoring already works” needs qualification: the ordinary prediction-object pathway uses training outcomes (finding 7). The hand-built simulation shim avoids that specific outcome-overwrite failure by construction, but is still affected by the weighting issues.
- A successful fresh fit and prediction in one R session is insufficient validation. Saved-model use can crash, and prediction can depend on the last unrelated model fitted (finding 1).
- Clear/rekey simulation caches after package fixes; reinstalling the package alone is insufficient (finding 14).

## Attribution to our changes versus the starting package

Compared against `145ac2f` (`b1c054a^`), the repository snapshot immediately
before the first LT implementation commit:

- **Ten LT implementation/integration defects:** findings 1, 2, 3, 5, 6, 9,
  10, 11, 12, and 13. Missing-time imputation and tuning already existed, but
  their demonstrated LT failures arise because we added entry times without
  adapting those paths. They are our integration responsibility, not evidence
  that the original right-censored package handled its supported input wrongly.
- **Two defects in our research/simulation code:** findings 4 and 14. These
  are not defects in the original R package.
- **Two inherited Brier defects:** findings 7 and 8. The pre-LT Brier function
  already overwrote test outcomes with training outcomes and misaligned subset
  predictions/outcomes. Re-running that historical R function confirms the
  original subset CRPS changes from 1.112106 to 0.2925387 on permutation, and
  equal-sized test predictions are evaluated against training times. Our shared
  setup carried the former behavior into the new AUC function; LT entry
  validation/indexing adds further consequences.

This establishes provenance relative to our starting repository. It does not
establish that these inherited defects remain in the latest CRAN release or
current upstream source, neither of which was checked for this attribution.

## Findings

### 1. P1 — Prediction uses missing or stale entry-time state

**Sites:** `src/entry.c:235`; `src/stack.c:484–496`.

`RF_entryTimeIn` is assigned by the grow entry point. The prediction entry point does not restore it from the fitted model, but stack initialization still dereferences it to construct entry indices.

Two reproduced consequences share this root cause:

1. Save a survival fit, start a fresh R process, load it, and call `predict()`: R terminates with SIGSEGV at address `0x8`. This also happens for an ordinary survival fit with `entry.time` omitted and default `save.memory=FALSE`. Restoration without new data and prediction from the forest subobject are affected too.
2. With `save.memory=TRUE`, predict from forest A, grow forest B with different entry times, and predict from unchanged A on unchanged data. The maximum survival difference in the fixture is **0.8423889**. Prediction reuses B's entry state when rebuilding node estimates.

**Origin:** LT plumbing introduced in `b1c054a`.

**Reproduce:** `repros/core-grow.R`, then `repros/core-predict.R` in a separate process; `repros/core-state-and-imputation.R` for state dependence. The prediction process deliberately crashes; logs are captured in `evidence/`.

**Fix direction:** persist and explicitly pass training entry times on every native grow/restore/predict invocation, with valid ownership and lifecycle. Do not depend on a pointer retained from the previous R call.

### 2. P1 — Missing-time imputation can return negative survival

**Sites:** `R/rfsrc.R:192`; `src/impute.c:279–281`; `src/survival.c:99–109`.

Initial validation skips missing exit times. Donor imputation does not constrain the imputed exit to be at or after entry. Terminal estimation then counts the event although the subject contributes no corresponding risk interval.

Fixture: 90 event subjects have missing exit time and entry 2; ten have observed event time 1 and entry 0. A one-node tree with `na.action="na.impute"` imputes time 1 for all missing exits. The fit returns **90 exits below entry, survival −9, and cumulative hazard 10**, without rejecting the data.

**Origin:** existing imputation was not adapted to the new LT constraint.

**Reproduce:** `repros/core-state-and-imputation.R`.

**Fix direction:** constrain or reject unsupported response imputation under LT and revalidate the event/risk relationship after imputation.

### 3. P1 — Restored training error and standalone VIMP ignore entry

**Sites:** `src/rfsrcUtil.c:459`; `R/vimp.rfsrc.R:70`; `R/subsample.rfsrc.R:184`; `R/utilities_subsample_bootstrap.R:33`; `R/utilities_subsample.R:148`.

The entry vector is supplied to concordance only in `RF_GROW`. `predict(f)` restoration and standalone `vimp(f)` use another mode, even when evaluating the original training observations.

Reproduced with identical OOB risk predictions: grow error **0.5990415** equals entry-aware concordance error, whereas restored `predict(f)` and `vimp(f)` report **0.6052801**, equal to entry-blind concordance error. This test uses `use.uno=FALSE` to isolate the pair-comparability correction.

Subsampling and double-bootstrap code can calculate reference VIMP through this path while their refitted forests calculate entry-aware grow-time importance. Forwarding entry times into refits therefore did not complete the confidence-interval integration.

**Origin:** incomplete LT integration, separate from finding 1's pointer problem.

**Reproduce:** `repros/r-interface.R`.

**Fix direction:** distinguish restoration of training outcomes from prediction on new outcomes and supply the correct entry vector for both. Recheck VIMP and interval construction end to end.

### 4. P1 — Cox survival-grid mapping corrupts existing simulation results

**Site:** `simulations/_common/sim_engine.R:48`.

The expression `ifelse(idx == 0, 0, bh$hazard[idx])` is incorrect when `idx` contains zeros. R drops zero indices from `bh$hazard[idx]`; `ifelse()` then recycles the shortened vector against the full condition. Later hazards are shifted, and values near the end can wrap around.

A deterministic fixture returns survival **0.2960 at t=9 but 0.8825 at t=12**, instead of remaining at **0.2960**. A survival curve can therefore increase sharply.

Reconstructed the original ten draws and both Cox phases for every saved scenario. The buggy implementation reproduces saved MAD values within **3e−16**, establishing that the saved results are affected. The diagnostic corrected mapping is `c(0, bh$hazard)[idx + 1L]`; this was evaluated in the reproduction script, not applied to production.

| Scenario | Affected replications, phase 1 / phase 2 | Saved phase-2 Cox MAD | Corrected-mapping phase-2 Cox MAD |
|---|---:|---:|---:|
| 01 | 8 / 8 out of 10 | 0.02783970 | 0.01021246 |
| 02 | 1 / 1 out of 10 | 0.10512830 | 0.10423138 |
| 03 | 0 / 0 out of 10 | 0.01852972 | 0.01852972 |
| 04 | 1 / 1 out of 10 | 0.08517660 | 0.08431690 |
| 05 | 0 / 0 out of 10 | 0.15242182 | 0.15242182 |
| 06 | 10 / 10 out of 10 | 0.10037781 | 0.08645374 |

The same helper generates training-sample Cox curves for outcome-based metrics, so the impact extends beyond the MAD table. These diagnostic values are not a replacement for a fully corrected rerun.

**Origin:** simulation helper, not native LT estimation.

**Reproduce:** `repros/simulation-cox.R`; add `--saved` to reconstruct the local saved runs without fitting forests or modifying results.

### 5. P1 — LT Brier construction does not recover its stated probability target

**Site:** `R/utilities_survival.R:473–505`.

The scorer combines individual entry-conditioned predictions, outcome-dependent truncation weights, and normalization over entered subjects. Under independent source `L,T ~ Uniform(0,1)`, retaining `L<T`, with no censoring:

| Supplied survival curve | Brier at t=.5 | Integrated score |
|---|---:|---:|
| True `S(t)=1−t` | 0.3797227 | 0.3631282 |
| Incorrect `S(t)=(1−t)^1.3` | **0.3558849** | **0.3476223** |

Lower scores are treated as better, so the implementation rewards a deliberately miscalibrated model.

There is an analytic mismatch independent of finite-sample noise. At fixed entry `L=l<s`, true entry-conditional survival is `(1−s)/(1−l)`. Cases receive weight `1/Fobs(T)` and controls `1/Fobs(s)`. Cases systematically receive larger weights, shifting the weighted squared-error minimizer below that true probability. Replacing the ECDF alone does not solve this construction problem.

**Qualification:** a weighted squared loss can target a deliberately tilted distribution. The demonstrated failure is to recover the stated source-population or entry-conditional Brier target; it is not a claim about every conceivable estimand.

**Origin:** new LT Brier specification/implementation.

**Reproduce:** `repros/metrics.R`; independently challenged by a second reviewer.

**Fix direction:** first choose the estimand, then derive the observation-process correction for that estimand, and validate with oracle predictions. Do not simply tune weights until the existing examples match.

### 6. P1 — Brier and AUC weights use a selected-sample entry CDF as a source CDF

**Sites:** `R/utilities_survival.R:388–394`, `490`, `597`.

The code treats the empirical distribution of retained entry times as the source-population entry distribution. Entry is fully observed for retained subjects, but those subjects were selected according to survival to entry.

For independent uniform source entry and event times, the true entry CDF at .5 is **.5**, while the retained entry CDF is `2l−l²`, hence **.75** at .5. One million simulated source draws give **.7495413** among retained subjects. The implementation's estimator converges to the wrong distribution for its claimed source-population weights.

**Origin:** new shared metric setup. Applies to both LT Brier and AUC. Separate from the score-construction problem in finding 5.

**Reproduce:** `repros/metrics.R`.

Primary context: [Li and Acharjee, Section 3.2.1](https://arxiv.org/html/2509.05693v1) uses truncation-adjusted estimation of source entry distributions; [Wang, Ying, and Xu, Section 2](https://pmc.ncbi.nlm.nih.gov/articles/PMC11648126/) distinguishes full and observed distributions under truncation. The elementary uniform counterexample above is self-contained.

### 7. P1 — Scoring a prediction object uses training outcomes

**Site:** `R/utilities_survival.R:309–323`.

Shared setup overwrites test `o$yvar` with `o$forest$yvar`, then constructs event information from the training outcomes. With 40 training subjects and 15 test subjects, both LT scorers reject the correct 15-value test entry vector. With equal sample sizes, they silently score against training times/status instead of the supplied test outcomes.

**Origin:** outcome overwrite is inherited from the original Brier setup; the new shared setup propagates it to AUC, and new LT validation adds the length failure.

**Reproduce:** `repros/metrics.R`.

**Fix direction:** separate training data used to estimate nuisance distributions from test outcomes used to evaluate predictions. A hand-built shim that replaces all training fields is not evidence that the normal prediction API is correct.

### 8. P2 — Brier subsetting misaligns outcomes, entry times, and curves

**Sites:** `R/utilities_survival.R:334–340`, `452–473`.

Predictions are subset, but the scoring loop indexes full-sample outcomes and entry times by the subset-local position. Merely reordering all four subjects from `1:4` to `4:1` changes CRPS from **1.2282174 to 0.3383005**. The set of subjects is unchanged.

**Origin:** outcome misalignment predates LT (also reproduced without entry); new entry handling repeats the same indexing mistake.

**Reproduce:** `repros/metrics.R`.

### 9. P2 — Accepted entry/event ties cause division by zero

**Sites:** `R/utilities_survival.R:490–492`, `597–598`, `639`.

The fitting API permits `entry == time`, but metric case weights use strict `mean(entry < time)`. With event times `1:4` and all entries equal to 1, this denominator is zero for the earliest event. Its Brier contributions are **Inf at every time**; AUC excludes its nonfinite weight and reports **NA** at the first event despite an observed event and controls.

**Origin:** inconsistent strict/inclusive conventions in the new metric logic.

**Reproduce:** `repros/metrics.R`.

### 10. P2 — Default NA omission rejects a normally aligned entry vector

**Sites:** `R/rfsrc.R:159`, `187`.

The data are reduced by `na.omit` before validating entry-vector length, but entries are not subset with the retained rows. A 120-row dataset with one missing predictor fits normally with 119 retained rows. Supplying its valid original 120-value entry vector instead fails length validation.

**Origin:** new per-observation argument not integrated with preprocessing.

**Reproduce:** `repros/r-interface.R`.

### 11. P2 — Tuning fails when its holdout branch is used with entry times

**Sites:** `R/tune.rfsrc.R:51–55`, `130–138`.

Tuning subsets observations for internal training but forwards the entire entry vector unchanged through `dots`. With `n=120`, `sampsize=30`, and `ntree.try=20`, ordinary tuning succeeds while entry-aware tuning fails entry-length validation. The branch is selected whenever `2*sampsize < n`.

**Origin:** tuning was not adapted to the new per-observation argument. The roadmap's “done for free” statement is incorrect for this path.

**Reproduce:** `repros/r-interface.R`.

### 12. P2 — Ordinary forests lose fast concordance scoring

**Sites:** `R/rfsrc.R:183–184`; `src/rfsrcUtil.c:459`; `src/survivalE.c:446`.

Omitted entry times are expanded to an all-zero vector. Grow-time performance nevertheless passes a non-NULL entry pointer, which unconditionally disables the Fenwick fast path. Thus ordinary survival OOB/importance scoring also falls back to quadratic pair enumeration.

A 10,000-subject illustration returned identical concordance values with NULL versus zero entry but took approximately **.002 versus .300 seconds** in the reviewer's run. Timing ratios vary by machine; the fallback condition is established from source.

**Origin:** concordance integration in `54737c3`.

**Reproduce:** `repros/performance-zero-entry.R`.

### 13. P2 — Unsupported splitting rules silently accept delayed entry

**Sites:** `R/rfsrc.R:317`; entry-bound checks exist in the default branch of `src/splitSurv.c:251` and `292` but not the alternative rules.

`logrankscore` and `bs.gradient` accept nonzero entry times without warning. For `logrankscore`, a fixture produces identical tree structure under zero and nonzero entry, while terminal survival changes. This combines an entry-blind partition with entry-aware leaf estimates.

**Origin:** missing scope guard. Previously documented as a concern; now runtime-confirmed, not newly discovered in this review.

**Reproduce:** `repros/r-interface.R` confirms silent acceptance; native reviewer independently compared tree structures.

### 14. P2 — Reinstalling fixed package code does not invalidate simulation caches

**Sites:** cached `loop` chunks in every `simulations/*/stage1_main.Rmd`; e.g. scenario 01 line 139.

The cache key includes replication counts and hashes of `setup.rds`, `dgp.R`, and `sim_engine.R`. It omits the installed package implementation. A change to native or R package code followed by reinstalling under the same version can therefore leave cached model results unchanged on re-knit.

An isolated knitr reproduction uses the same key structure: the first knit reads a package function returning **.111**. After replacing its implementation, a direct call returns **.999**, but re-knitting still reports **.111**. No package namespace is changed in the parent process or installation; the reproduction is isolated.

**Origin:** simulation caching design.

**Reproduce:** `repros/cache.R`, using temporary files only.

**Fix direction:** include a package build/source fingerprint in cache provenance, or explicitly invalidate all affected caches after changes. Package version alone is insufficient while development uses the same version number.

## Verification and limits

- Built the current R/C source with `R CMD INSTALL --preclean` from a copied package in a temporary directory, using a separate temporary library. The normal installed package was not replaced.
- Re-ran the portable reproduction suite against that build. `core-predict.R` intentionally exits with SIGSEGV; all other scripts completed successfully while exhibiting/asserting the reviewed failures. See `evidence/run-summary.txt` and individual logs.
- Metric reproductions additionally source the current `R/utilities_survival.R` into a namespace-parent environment so their source provenance is explicit.
- Saved-run impact reconstruction uses local ignored `setup.rds` and `results_phases.rds` files. It does not rerun forests or overwrite any results.
- This is not an exhaustive correctness proof. No sanitizer, Windows build, or full package check was run. Statistical validity of every concordance weighting option and alternative censoring estimator remains outside the established evidence.

The reproduction scripts characterize current failures, not desired behavior. Some deliberately assert that the bug occurs; they will need to be rewritten as regression tests when fixes are made.
