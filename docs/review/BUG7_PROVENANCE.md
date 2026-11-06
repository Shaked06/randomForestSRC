# Bug 7: provenance and tuning scope

Verified 2026-11-02. This corrects any implication that the current upstream
repository necessarily has the defect seen in our older starting snapshot.

## Source versions

- Our pre-LT baseline: `145ac2f8692a724fcd451960dd4ef4fbe74ff08a`, authored
  2026-04-19 by the upstream maintainer. Its original `get.brier.survival()`
  assigns `o$forest$yvar` to `o$yvar`, then constructs evaluation outcomes
  from that overwritten field. This source predates our LT work.
- Current upstream main:
  [30d95e206b8e287863afb55970014497e3803a8f](https://github.com/kogalur/randomForestSRC/commit/30d95e206b8e287863afb55970014497e3803a8f),
  commit dated 2026-10-27. We queried GitHub's commits API and downloaded
  source pinned to that commit, rather than trusting stale remote refs.
- Its [survival scoring source](https://github.com/kogalur/randomForestSRC/blob/30d95e206b8e287863afb55970014497e3803a8f/R/utilities_survival.R#L478)
  explicitly separates `grow.yvar` from `eval.yvar`. The former estimates
  censoring; the latter supplies validation outcomes. Unlabeled evaluation
  data produce a clear error. This check establishes the corrected behavior
  in that commit, not the date when the correction was introduced.

## Reproduction

`repros/bug7-provenance.R` fits an ordinary forest with 40 uncensored
training subjects and creates genuine prediction objects on 40 test subjects.
Two test datasets have identical predictors but different outcomes: A has
all failures after the prediction grid, B has all failures before it.
Predicted survival matrices are identical, as they should be.

With no censoring, independent Brier references are `mean((1-S)^2)` for A
and `mean(S^2)` for B, at each prediction time. We load unchanged pre-LT
and current upstream R scoring files into separate namespace-parent
environments and apply them to the same prediction objects.

| Scorer | Mean Brier A | Mean Brier B | Evaluation outcomes used correctly? |
|---|---:|---:|---|
| Pre-LT baseline | 0.1698360 | 0.1698360 | No; matches training outcomes |
| Local LT branch at `42d7a66` | 0.1698360 | 0.1698360 | No; matches training outcomes |
| Current upstream scoring source | 0.3477752 | 0.3686155 | Yes; matches independent references |

Machine-readable results: `evidence/bug7-provenance.csv`.
Current upstream also rejects a genuine prediction object lacking outcomes.

Scope: the prediction objects/native forest come from our isolated local
build; the upstream R scoring file is unchanged. This isolates the R scoring
logic. We did not build the complete current upstream package or test CRAN.
The fixture uses no LT and no censoring, so the inherited outcome mismatch
is demonstrated independently of the new LT weights or conditional target.

## Hyperparameter tuning

Both local and inspected upstream `tune()` and `tune.nodesize()` evaluate
`get.mv.error(fit, TRUE)` or `get.mv.error(predict(...), TRUE)`.
[Upstream tuning source](https://github.com/kogalur/randomForestSRC/blob/30d95e206b8e287863afb55970014497e3803a8f/R/tune.rfsrc.R#L98).
`get.mv.error()` reads the native `err.rate`; it does not call the Brier helper.
Thus this particular Brier outcome-overwrite defect is not in the built-in
tuning objective's call path. This does not validate every tuning option
or resolve the separate LT tuning-entry bug 11.

For the fixture, native new-data concordance errors are 0.4429487 for A
and 0.5570513 for B. Both match independent explicit pair counting using
the respective test outcomes. A custom tuning loop calling the affected
local Brier/AUC helpers on prediction objects can still be affected.

## Running the provenance script

Prepare a temporary directory with:

1. `pre-lt-utilities_survival.R` from
   `git show 145ac2f8692a724fcd451960dd4ef4fbe74ff08a:R/utilities_survival.R`.
2. `utilities_survival.R` downloaded from the pinned upstream commit above.
3. Run `Rscript docs/review/repros/bug7-provenance.R /path/to/source-directory`
   with `R_LIBS` selecting the local build being examined.

The script asserts the baseline defect and upstream correction. Its local
row is diagnostic so that it remains useful after a local fix. The fixture
is deterministic on the recorded build; numeric forest results can change
with package/native implementation, without changing the reference checks.

SHA-256 of the inspected files:

- `pre-lt-utilities_survival.R`: `f6ac635d4f86cecf2fd9a08010acea056847513278aaddb39007150ade0fcde1`
- `utilities_survival.R`: `1812ac29a2e4e8a9e389348be4ca3fd7721da0e52595e076873dec18d4bb27d4`
- `tune.rfsrc.R`: `17408db8df16c7c510a15bad97219160ab7c0044dd410707bdcfb7c0b4751744`
- `tune.nodesize.rfsrc.R`: `be64ca0e462f4a9f1422de04694fd0e3d56ccdd5a543bc2bc668ede1ef0c6042`
