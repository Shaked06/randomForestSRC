# Left-Truncation Support Across the Evaluation Toolkit

Status of every evaluation tool listed earlier (see the project conversation),
against whether it needs — and has — a left-truncation-aware version. Two
tools already have their own detailed math spec in this folder:
`c_index_left_truncation.md` (implemented) and `brier_score_left_truncation.md`
(not yet implemented).

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
| `get.brier.survival()` | Brier Score (IPCW) | **Spec written, not implemented** | `brier_score_left_truncation.md` has the full math (risk-set filtering *and* conditional-survival reweighting — a bigger change than C-index's pair filter). Natural next candidate. |
| `get.auc()` | Time-dependent AUC | **Needs a spec — none written yet** | Time-dependent AUC has the same risk-set-at-time-*t* issue as Brier score. No design work done. |
| `predict()`'s test-set error/C-index/VIMP | (test-set versions of the above) | **Out of scope (explicit decision)** | `predict()` has no entry-time channel at all for new data — no R argument, no `RF_fentryTime` global. Making this truncation-aware needs new plumbing roughly the size of this session's whole C-index change, on its own. This is exactly why our simulations score C-index on the *training* sample instead of the test set. |
| `max.subtree.rfsrc()` | Maximal Subtree (minimal depth importance) | **Not applicable** | Purely structural (tree depth/topology), not outcome-pair-based. Already correct, since it reads a tree that was grown with entry-time-aware splitting. |
| `partial.rfsrc()` / `plot.variable.rfsrc()` / `plot.survival.rfsrc()` / `get.tree.rfsrc()` | Partial dependence / marginal-effect plots / survival-curve plots / single-tree view | **Not applicable** | Curve/structure visualization for one subject or one tree at a time — no cross-subject risk set involved. |

## Suggested order, if continuing this work

1. ~~Verify `holdout.vimp.rfsrc()` and `subsample.rfsrc()`~~ — **done**.
   `holdout.vimp.rfsrc()` confirmed free; `subsample.rfsrc()` confirmed
   *not* free — it's a real gap (see table above), not just an unverified
   assumption.
2. **Fix `subsample.rfsrc()`**, if VIMP confidence intervals under
   truncation become something the project actually needs — add
   `entry.time` to its recycled parameter list and subset it per-sample
   (`entry.time[pt]`) alongside the row subsampling itself.
3. **Implement Brier score** — the spec already exists, and it's the other
   metric our own `sim_engine.R` would plausibly want (MAD-vs-truth is
   already Brier-score-shaped; a real, outcome-based Brier score would be a
   natural third scorer alongside MAD and C-index).
4. **AUC** only if a concrete need shows up — no spec exists yet, and it's
   the least central of the three to this project's actual questions.
5. **`predict()` entry-time plumbing** is the biggest remaining piece, and
   only worth it if test-set (not training-sample) truncation-aware scoring
   becomes something the project actually needs.
