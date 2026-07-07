# LTRC-RSF Progress Log

Working log for the left-truncation extension described in `TASK.md`. This
entry covers the first working session: baseline repair, the shared
plumbing, both edit sites, and a smoke test.

## 1. Baseline repair (prerequisite, not in TASK.md)

`main` (and this branch, before any edits) did not compile. 46 files carry
unresolved template placeholders — `${trace.token}`, `${memor.token}`,
`${stack.token}` — left over from kogalur's private release tooling, which
normally substitutes them (`""` for a debug build, `//` for release) before
publishing a real release. Confirmed via `git fetch upstream`: identical to
`kogalur/main`, so this isn't fork-specific breakage. The last clean tagged
release (`3.3.4` / internal `3.4.4`) predates current `main` (`3.6.2`) by two
minor versions, so it wasn't used as a base.

Fix: mechanical `//`-replacement of all three tokens across 49 files (8,952
lines, mechanical 1:1 swap, spot-checked against several files to confirm
every occurrence is confined to debug/diagnostic-only blocks). Reproduces
what a standard release build would contain (tracing/memory-accounting
disabled). R 4.6.1 installed locally (Homebrew formula — the GUI cask needs
`sudo`, unavailable here) to actually compile and test, rather than relying
on static review alone.

Verified: clean `R CMD INSTALL`, and a real `rfsrc(Surv(time,status)~., data=veteran)`
fit runs and produces sane OOB error / CRPS.

## 2. Shared plumbing (`entry.time`)

New optional `rfsrc(..., entry.time = L)` argument, threaded R → `.Call` →
C, defaulting to `rep(0, n)` when omitted (reproduces standard right-censored
behavior exactly).

- **R side** (`R/rfsrc.R`): new argument; validated for length/NA/non-negativity,
  and (see §4) that `entry.time <= time` for every subject; appended as the
  last `.Call("rfsrcGrow", ...)` argument.
- **C entry point** (`src/entry.c`, `src/entry.h`, `src/R_init_randomForestSRC.c`):
  new `SEXP entryTime` parameter appended (not inserted, to avoid renumbering
  every existing positional arg), unpacked into a new global `RF_entryTimeIn`.
- **Core globals** (`src/internal.c`, `src/external.h`): `RF_entryTimeIn`
  (raw, shared) and `RF_entryTime` (per-tree `double**`, mirrors `RF_time`/
  `RF_status`).
- **Grid-index lookup** (`src/impute.c`, `src/impute.h`): new
  `updateEntryTimeIndexArray()` — a **lower-bound** search against the
  master time grid (`RF_masterTime[]`), unlike the existing
  `updateTimeIndexArray()` which requires an *exact* match. Entry times
  generally don't land exactly on an observed event/censoring time, so the
  exact-match helper couldn't be reused as-is. Populates a new
  `RF_masterEntryTimeIndex` (mirrors `RF_masterTimeIndex`).
- **Wiring** (`src/stack.c`): `RF_entryTime`/`RF_masterEntryTimeIndex`
  constructed alongside `RF_time`/`RF_status`/`RF_masterTimeIndex` in
  `stackMissingArraysPhase1`, with matching cleanup in
  `unstackTimeAndSubjectArrays`/`unstackMissingArrays`.

## 3. Site 1 — split rule

**Correction to TASK.md**: the document names `getCustomSplitStatisticSurvival`
(`src/splitCustom.c`) as the "CONFIRMED" edit site. That function is actually
one of 16 interchangeable slots in a generic dispatch table
(`customFunctionArray[4][16]`), constrained by a single fixed C function-pointer
type (`customFunction`, `src/split.h`) shared across all four families
(classification/regression/survival/competing-risk) — it only runs when a
user explicitly opts into `splitrule="custom"`. TASK.md's own "Path decision"
already ruled that route out (can't reach Site 2 through it), so the real
edit site has to be the **default** splitter.

Traced from `rfsrc()`'s default `splitrule="logrank"`:

- `src/splitUtilSurv.c`, `getSplitEventAndRisk()` — whole-node at-risk table,
  built once per node. Added the entry-time lower bound to the existing
  suffix condition:
  ```c
  if ((eventTimeIndex[i] <= RF_masterTimeIndex[treeID][...]) &&
      (eventTimeIndex[i] >= RF_masterEntryTimeIndex[treeID][...])) {
    parentAtRisk[i]++;
  }
  ```
- `src/splitSurv.c`, `logRankNCR()` — per-candidate-cutpoint LEFT at-risk,
  built incrementally as the split point sweeps sorted covariate values (two
  code paths: categorical `factorFlag==TRUE`, recomputed from scratch each
  candidate; continuous `factorFlag==FALSE`, incremental). Both gated the
  existing `nodeLeftAtRisk[tIndx]++` on the same added lower-bound check.
  Event-count logic (`nodeLeftEvent`) is untouched — unaffected by
  truncation, per TASK.md's core principle.

Turned out simpler than TASK.md anticipated: the design doc's reverse-cumsum
concern was accurate for the pedagogical `splitCustom.c` copy, but the real
production code never builds a true cumulative sum — it re-derives at-risk
membership per subject directly, so the fix is a loop-bound change, not the
diff-array rewrite originally designed.

Deliberately left untouched (out of scope): `SURV_LRSCR` (a different split
statistic), `logRankCR` (competing risk), `brierScoreGradient1` and its
helpers (dead code / different splitrule, confirmed via call-site search).

## 4. Site 2 — terminal-node estimator

`src/survival.c`, `getAtRiskAndEventCount()` — all three code branches
(`OPT_MEMB_OUTG` / `OPT_MEMB_INCG` / default) changed from
`for (j = 1; j <= RF_masterTimeIndex[treeID][ii]; j++)` to
`for (j = RF_masterEntryTimeIndex[treeID][ii]; j <= RF_masterTimeIndex[treeID][ii]; j++)`.
`getLocalRatio`/`getLocalNelsonAalen`/`getNelsonAalen` consume the corrected
`atRiskCount` unchanged — inherit the fix automatically. The predict-time
path (`OPT_TERM_INCG` branch) is an intentional no-op that reuses grow-time
values rather than recomputing; correctly out of scope.

**Real bug found and fixed along the way**: the `entry.time <= time`
validation was first written in C (`stack.c`, `stackMissingArraysPhase1`).
Turned out that whole block is skipped whenever R's internal `data.pass`
optimization is `TRUE` — which R auto-sets whenever the dataset has no
missing values (the common case). Invalid data (`entry.time > time`) sailed
straight through into tree-growing and crashed with a segfault, surfacing
through an unrelated pre-existing consistency check
(`getLocalRatio`'s "Zero At Risk Count", never reachable before truncation
made empty risk-sets possible). Fixed by moving the validation to R
(`rfsrc.R`), which runs regardless of `data.pass` — it's the actually
authoritative layer for a normal in-memory call.

## 5. Verification

Ad hoc `Rscript` checks (not yet a permanent test file — still open, see
below), on `randomForestSRC`'s bundled `veteran` dataset and a fresh
synthetic dataset (`demo/ltrc_smoke_test.R`):

- **Baseline regression** (the check TASK.md calls for): `entry.time=0`
  reproduces the standard right-censored forest **bit-for-bit** — `err.rate`,
  full tree structure (`forest$nativeArray`), `survival`, and `chf` all
  identical, on two independent datasets.
- **Live effect**: a valid nonzero `entry.time` measurably changes both
  splits and terminal-node survival curves, no NaN/Inf.
- **Validation**: `entry.time` rejected cleanly (R-level error, no crash)
  when wrong length, containing NA, negative, or exceeding the subject's own
  observed time; the boundary case `entry.time == time` is accepted and
  produces finite output.
- **Stability**: heavy truncation (`entry.time` close to `time` for most
  subjects) runs without crashing or producing non-finite output.
- **End-to-end demo** (`demo/ltrc_smoke_test.R`): fits both a standard and an
  LTRC-aware RSF on the same synthetic left-truncated dataset, confirms they
  diverge, confirms `entry.time=0` still collapses to the standard fit, and
  confirms `predict()` on new data produces valid survival probabilities.

## What's left (per `TASK.md` and this session)

- `stage0_setup.Rmd` needs a left-truncation entry mechanism added to its DGP
  (currently right-censoring only) before a real `stage1`-style Monte Carlo
  comparison (standard RSF vs. LTRC RSF vs. Cox-LT) can run.
- A permanent test file (not just ad hoc scripts / the demo) for regression
  safety.
- OOB / prediction-error metrics that respect delayed entry (LT-aware
  C-index, integrated Brier score with delayed-entry correction) — TASK.md
  §8–9, explicitly deferred there as "later" work.
