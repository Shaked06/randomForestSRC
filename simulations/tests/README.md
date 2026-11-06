# Simulation regression tests

Run from the repository root:

```sh
Rscript simulations/tests/cox_survival_matrix.R
Rscript simulations/tests/package_cache.R
Rscript simulations/tests/package_provenance.R
```

The test compares the shared Cox prediction helper against `survival::survfit`
for ordinary and delayed-entry Cox models, tied observations, early censoring,
and grids before, within, and after follow-up. It also checks dimensions,
probability bounds, and monotonicity. It does not rerun simulations or write
saved results or caches.

The cache test builds a tiny R/C package in a temporary library and knits a
small document in fresh R processes using the actual stage1 cache expression.
It checks real cache hits with unchanged code, and recomputation after R-only
or native-only changes under the same package version and metadata. It also
checks that all six stage1 documents include package provenance. It needs
`knitr` and an R-compatible C compiler. All its installs, results, and caches
are temporary; existing simulation outputs are untouched.

The provenance test requires an installed current fork. It checks worker
library selection, rejection and cleanup on fingerprint mismatches, and
provenance in serialized results. A small real experiment is compared with
serial replications; existing setups, saved simulations, and caches are not
changed. Its workers use local sockets.

These tests cover research code outside the R package and are run separately
from package tests in the root `tests/` directory.
