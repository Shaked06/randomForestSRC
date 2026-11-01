# Simulation regression tests

Run from the repository root:

```sh
Rscript simulations/tests/cox_survival_matrix.R
```

The test compares the shared Cox prediction helper against `survival::survfit`
for ordinary and delayed-entry Cox models, tied observations, early censoring,
and grids before, within, and after follow-up. It also checks dimensions,
probability bounds, and monotonicity. It does not rerun simulations or write
saved results or caches.

These tests cover research code outside the R package and are run separately
from package tests in the root `tests/` directory.
