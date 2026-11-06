# Additional issue exposed while fixing saved-model prediction

`partial(..., oob = FALSE)` has separate unsafe normalization. In
`src/partial.c`, `summarizePartialCalculations()` unconditionally reads
`RF_oobEnsembleDen`, even though this call requests the full ensemble. The
same assignment is present in the pre-LT source (`b1c054a^`). With no OOB
ensemble allocated for this invocation, the pointer can contain stale state.

During fix 1 testing, repeated calls or fresh-process calls returned different
values, and in one run infinite values. Changing allocation patterns can expose
this pre-existing defect. Default `oob = TRUE` partial dependence passed the
saved-model regression test.

This issue is intentionally not included in the first fix. It needs its own
reproduction, regression test, and commit before calling full-ensemble partial
dependence validated.
