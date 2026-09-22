# Left-Truncation Support Across the Evaluation Toolkit

Status of every evaluation tool listed earlier (see the project conversation),
against whether it needs — and has — a left-truncation-aware version. Three
tools have their own detailed math spec in this folder:
`c_index_left_truncation.md`, `brier_score_left_truncation.md`, and
`auc_left_truncation.md` — all three now implemented.

## Why some tools need this and others don't

A metric needs truncation-awareness only if it compares **pairs or risk sets
across subjects** at a point in time — that's exactly where an unadjusted
metric can compare subject `i`'s outcome against subject `j` when `j` hadn't
even entered the study yet (the bias `c_index_left_truncation.md` describes).
Metrics that only look at **one subject's own predicted curve**, or at
**tree structure**, don't have this problem — there's no cross-subject risk
set to get wrong. `rfsrc()`'s tree-*growing* itself already handles entry
times correctly (that's the original fork feature, separate from all the
scoring tools below).

## Status table

| Tool | Official name | Status | Notes |
|---|---|---|---|
| `get.cindex()` | C-Index (Harrell's concordance) | **Done** | This session's work: `entry=` argument, filters pairs to `entry[later subject] ≤ event time`. Verified against `c_index_left_truncation.md`'s own reference implementation. |
| `rfsrc()` training OOB error | Out-of-bag error rate | **Done** | Same native fix — `getPerformance()` now reads the existing `RF_entryTime` global automatically whenever a forest is grown with `entry.time=`. |
| `vimp.rfsrc()` (grown with `importance=`) | Variable Importance (VIMP) | **Done, for free** | Verified: VIMP's error computation (`importance.c` → `summarizePerturbedPerformance()` → `getPerformance()`) is the *same* call site we already patched, gated by the same `mode == RF_GROW` check. No extra code needed. |
| `tune.rfsrc()` | Hyperparameter tuning (`mtry`/`nodesize`) | **Done, for free** | Selects `mtry`/`nodesize` by OOB error — inherits the fix transitively, same reasoning as VIMP. (Separately, we already found `tune()`'s OOB-only objective isn't a good fit for our project regardless of truncation — see sim02's `stage0_setup.Rmd`.) |
| `print.rfsrc()` / `plot.rfsrc()` | Model summary / error-rate plot | **Done, for free** | Both just display `err.rate`, which is now computed correctly at grow time. |
| `holdout.vimp.rfsrc()` | Holdout Variable Importance | **Done, for free** | Verified: its R wrapper forwards `entry.time` through `...` into its own `rfsrc()` calls, and its C-level performance computation (`summarizeHoldoutBlockPerformance()`) calls the same `getPerformance()` we patched. |
| `subsample.rfsrc()` | VIMP confidence intervals (subsampling) | **Not done — real gap, not just unverified** | Checked the source: it takes an *already-fitted* forest and refits on row subsamples using a hand-picked parameter list (`ntree`, `mtry`, `nodesize`, `bootstrap`, ...) pulled back off that object. `entry.time` is not one of them. Even a forest originally fit with `entry.time=` loses it on every subsample refit inside this function — confirmed by grep: `entry.time` appears nowhere in the R source except `rfsrc.R` itself. Fixing this means adding `entry.time` to the recycled parameter list *and* subsetting it in sync with each subsample's row indices (`entry.time[pt]`), not just passing it through. |
| `get.brier.survival()` | Brier Score (IPCW) | **Done** | Modified in place (pure R, no C involved). Implements all three adjustments from `brier_score_left_truncation.md`: risk-set restriction (`R_L(t)`, via `colMeans(na.rm=TRUE)` over cells masked `NA` when `entry > t`), conditional-survival reweighting (`S(t\|X)/S(entry\|X)`), and the truncation weight `1/G_L(...)` alongside the existing censoring weight. `entry.time = NULL` (default) is byte-identical to the untouched function. Verified against a fully hand-computed example (exact match to 6 decimal places) after an initial bug (a boundary-value assumption borrowed from the censoring-weight code that didn't transfer to the truncation weight) was caught and fixed. One caveat found and left alone as out of scope: `cens.model = "rfsrc"` was already non-deterministic between calls before this change (its internal censoring-forest refit has no fixed seed) — confirmed unrelated to left truncation; the default `cens.model = "km"` path is fully deterministic and is what's been verified. |
| `get.auc.survival()` | Time-dependent AUC (cumulative/dynamic) | **Done** | New function — see `auc_left_truncation.md`. Same risk-set-at-time-*t* issue as the Brier score, and the same three adjustments: risk-set restriction to `entry ≤ t` (which provably only ever drops *controls*, since a case always has `entry ≤ T ≤ t`), conditional marker `1 - S(t\|X)/S(entry\|X)`, and joint censoring/truncation weights at `T_i^-` for cases and `t` for controls. Verified against a literal double-loop transcription of the spec (agreement to 1e-13) and against two null cases: an identical-curve marker returns *exactly* 0.5, and a permuted marker centers on 0.5 (mean 0.506 over 40 draws, p = 0.36). Note it is **higher-is-better**, unlike every other metric here. |
| `get.auc()` | Multiclass classification AUC (Hand & Till 2001) | **Not applicable** | Listed in earlier versions of this table as "time-dependent AUC" — that was wrong. It scores classification probabilities, has no time axis and no risk set, so left truncation cannot apply to it. The survival AUC is the separate `get.auc.survival()` above. |
| `predict()`'s test-set error/C-index/VIMP | (test-set versions of the above) | **Out of scope (explicit decision)** | `predict()` has no entry-time channel at all for new data — no R argument, no `RF_fentryTime` global. Making this truncation-aware needs new plumbing roughly the size of this session's whole C-index change, on its own. This is exactly why our simulations score C-index on the *training* sample instead of the test set. |
| `max.subtree.rfsrc()` | Maximal Subtree (minimal depth importance) | **Not applicable** | Purely structural (tree depth/topology), not outcome-pair-based. Already correct, since it reads a tree that was grown with entry-time-aware splitting. |
| `partial.rfsrc()` / `plot.variable.rfsrc()` / `plot.survival.rfsrc()` / `get.tree.rfsrc()` | Partial dependence / marginal-effect plots / survival-curve plots / single-tree view | **Not applicable** | Curve/structure visualization for one subject or one tree at a time — no cross-subject risk set involved. |

## Suggested order, if continuing this work

1. ~~Verify `holdout.vimp.rfsrc()` and `subsample.rfsrc()`~~ — **done**.
   `holdout.vimp.rfsrc()` confirmed free; `subsample.rfsrc()` confirmed
   *not* free — it's a real gap (see table above), not just an unverified
   assumption.
2. ~~Implement Brier score~~ — **done**. Not yet wired into `sim_engine.R`
   as a third scorer alongside MAD and C-index — that's a separate step,
   same shape as the C-index wiring, whenever wanted.
3. ~~Implement time-dependent AUC~~ — **done**. Spec in
   `auc_left_truncation.md`, implemented as `get.auc.survival()`. Not yet
   wired into `sim_engine.R` as a fourth scorer — same shape as the Brier
   wiring, but note it is higher-is-better, so the engine (which assumes
   lower-is-better throughout) needs `1 - auc`.
4. **Fix `subsample.rfsrc()`**, if VIMP confidence intervals under
   truncation become something the project actually needs — add
   `entry.time` to its recycled parameter list and subset it per-sample
   (`entry.time[pt]`) alongside the row subsampling itself.
5. **`predict()` entry-time plumbing** is the biggest remaining piece, and
   only worth it if test-set (not training-sample) truncation-aware scoring
   becomes something the project actually needs.
