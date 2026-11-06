# Additional issue exposed while fixing saved-model prediction

Update: bug 15 now has an approved native fix and passing regression tests,
including the reproduced fresh-process crash. See `../BUGS.md` for the
local commit status. The explanation below records why it was deferred
from fix 1.

`partial(..., oob = FALSE)` has separate unsafe normalization. In
`src/partial.c`, `summarizePartialCalculations()` unconditionally reads
`RF_oobEnsembleDen`, even though this call requests the full ensemble. The
same assignment is present in the pre-LT source (`b1c054a^`). With no OOB
ensemble allocated for this invocation, the pointer can contain stale state.

During fix 1 testing, repeated calls or fresh-process calls returned different
values, and in one run infinite values. Changing allocation patterns can expose
this pre-existing defect. Default `oob = TRUE` partial dependence passed the
saved-model regression test.

This issue was intentionally not included in the first fix. Its separate
regression is now `tests/partial-full-ensemble.R` at the repository root.
It compares full predictions with ordinary modified-covariate predictions
and independently checks OOB averaging from per-tree predictions.
