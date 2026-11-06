# Upstream integration review

Prepared and approved 2026-11-06. This local merge incorporates the tested
integration. No GitHub push is authorized.

## Exact Git state

| Role | Version |
|---|---|
| Original repository | `https://github.com/kogalur/randomForestSRC.git` (`upstream`) |
| User's fork | `https://github.com/shaked06/randomForestSRC.git` (`origin` in the working repository) |
| Working LT tip | `42d7a66b93816edb05d483857cb215b6a2f66c52` |
| Common ancestor | `145ac2f8692a724fcd451960dd4ef4fbe74ff08a` |
| Fetched upstream tip | `30d95e206b8e287863afb55970014497e3803a8f` |
| Histories beyond ancestor | 21 LT-branch commits and 6 upstream commits |
| Package versions | 3.6.2 before integration; 3.9.7 in the candidate |

The integration was prepared on `integrate-upstream-2026-11-06` in an
isolated checkout. This merge retains the LT tip and upstream tip as its two
parents. All conflicts were resolved and tested before approval. Bringing
this approved commit into the thesis branch preserves pending research edits
and saved simulation outputs; no stash or history rewrite is needed.

Candidate checkout:

```
/var/folders/4v/ydm4_m3n3yb12w3jmtncgx4r0000gn/T/rfsrc-upstream-integration-lrkerrxk/repo
```

A durable `upstream-integration.patch` and an external manifest are kept
alongside this review. The manifest records the reviewed parents/tree, patch hash, checkout
location, and the approved commit after creation. The binary patch is against the LT tip and
includes new/deleted files, so it preserves the result independently of the
temporary checkout. To reconstruct it, create a clean checkout at the pinned
LT tip and apply it there with `git apply --index`; re-establish the pinned
upstream merge parent when preparing the reviewed merge commit. Do not apply
it directly over the user's uncommitted research files.

## Reconciliation

The initial trial flagged 59 files: 50 C/header, 5 R, and 4 namespace/manual
files. Most C conflicts were debug cleanup. In 35 native files, local changes
only converted the baseline trace-token lines to comments, while upstream
removed those lines. Normalizing those lines allowed 42 C/header files to
merge without remaining conflict blocks; the residual debug blocks and
RNG/entry interfaces were then reconciled explicitly.

- Preserve upstream's new RNG protocol and our training-entry ownership.
  Native grow and predict now take 40 and 65 arguments respectively, with
  RNG protocol, thread count, and entry vector in matching R/C order.
- Retain LT split/terminal risk sets and saved-entry restoration/performance
  fixes. Ordinary forests are compared with a pristine upstream build.
- Base ordinary survival scoring on upstream's corrected separation of grow
  and evaluation data. Apply the same separation to LT scoring: validation
  outcomes/entries determine losses; training outcomes and saved training
  entries determine censoring probabilities.
- Keep both public AUC names: upstream's `get.auct.survival()` and our thesis
  compatibility helper `get.auc.survival()`. The latter retains `auc` and
  descriptive `iauc` fields used by the simulations.
- Preserve LT entry propagation in subsampling while retaining upstream's
  sampling validation and missing-event checks.
- Retain upstream's replacement help topics and add LT arguments and scope.
  Ordinary confidence intervals remain available through the upstream API;
  LT confidence intervals are explicitly rejected rather than implied.

## Numerical changes to understand before committing

The `km` censoring option now uses upstream's product-limit Kaplan–Meier
estimator. The LT version uses delayed-entry risk sets and removes failures
before censoring at tied times, matching upstream's ordering. This replaces
our earlier exponential Nelson–Aalen censoring survival. Censored scores can
change even for fixed predictions; hand-computed tests and independent
`survfit` references were updated for this estimator deliberately.
The uncensored oracle examples still have their exact expected losses/AUC.

Upstream also adds a new default RNG protocol. New forests can therefore
differ from historical fits under the same seed. Saved forests use their
stored protocol, and forests predating that metadata use the legacy protocol.
This behavior is tested; no claim is made that selecting the legacy protocol
undoes every other upstream algorithm change.

## Verification

1. Clean isolated source build and `R CMD check`: **Status OK**. Examples,
   manual PDF, and vignettes were skipped. Unavailable optional suggested
   packages were informational; they were not installed for this check.
2. All **8 package regression files** pass, including the existing 7 LT
   tests and `tests/survival-evaluation-outcomes.R` for bugs 7–8. The latter
   checks equal/different-size validation samples, independent losses,
   training-only censoring, validation entries, subsets, absent outcomes,
   both AUC names, custom time grids, and forest-based censoring.
3. Cox grid regression passes: 4 fits, 7 grid cases, one/multiple subjects.
4. All **6 simulation smoke cases** pass with 200 retained subjects,
   25 trees, 30 population test subjects, and independent zero-entry checks.
   These are reduced-size integration checks, not rerun thesis experiments.
5. **12 ordinary profiles per build** agree with pristine upstream within
   `1e-12`: 2 RNG protocols, survival/regression/classification, and complete/
   missing predictors. Grow, new-data prediction, restoration, importance,
   and survival scores are compared. Explicit-zero integration profiles also
   agree. Saved protocol behavior survives changing the session default.
6. **4 saved pre-integration forests** (zero/positive entry crossed with
   both memory settings) preserve restored and new-data predictions within
   `1e-12` after loading in the integrated package. They have no RNG metadata,
   exercising the legacy fallback while the session uses the new default.
7. Bugs 10/11/13 and the stale-cache finding were reproduced on the merged
   build. The zero-entry slow-path condition remains visible in native code;
   the full-ensemble partial issue still requires its dedicated test.

Evidence is in `docs/review/evidence/upstream-*`; text log trailing blanks
are normalized for readability. Reproduction scripts are under
`docs/review/repros/upstream-*`. The source package check copies package
files only; research reports and the full saved Monte Carlo outputs are
outside its scope.

## Bug status and next work

- Bugs 1–6 and 9 retain their previous fixes.
- Bugs 7 and 8 are resolved by this merge, combining upstream corrections
  with the LT adapter.
- Bugs 10–14 remain open. The package version change does not replace a
  source fingerprint for future simulation-cache invalidation.
- Bug 15 remains a dedicated reproduction/test task. Passing default OOB
  partial tests does not establish full-ensemble partial correctness.
- Historical simulation results and rendered reports have not been
  regenerated. The KM/RNG changes and remaining cache issue must be considered
  when the final thesis simulations are rerun.

The user approved the **local merge commit** after review. Package sources
match the tested candidate; approval-time edits update documentation status.
The external manifest records the reviewed and committed trees.
