# Running the review reproductions

Read `BUG_REVIEW.md` for findings and `evidence/` for captured outputs.
For current resolution status and fix commits, see `../BUGS.md`. The review
reproductions describe the original failures and can stop failing or error
after a fix; the package regression tests in `tests/` define expected behavior.

Run from the repository root with an installation of this branch:

```sh
Rscript docs/review/repros/r-interface.R
Rscript docs/review/repros/core-state-and-imputation.R
Rscript docs/review/repros/metrics.R
Rscript docs/review/repros/simulation-cox.R --saved
Rscript docs/review/repros/performance-zero-entry.R
Rscript docs/review/repros/cache.R
```

To select an isolated installation, prefix each command with
`R_LIBS=/absolute/path/to/library`. The `--saved` option needs the ignored
local simulation setup and result RDS files; omit it for the standalone
Cox helper counterexample.

The saved-model bug must be tested in **separate R processes**. The second
command below intentionally crashes R on the reviewed source:

```sh
Rscript docs/review/repros/core-grow.R /tmp/review-survival-fit.rds
Rscript docs/review/repros/core-predict.R /tmp/review-survival-fit.rds
```

Capture stderr when running the crash reproduction to avoid a large native
traceback in the terminal. Do not run it in an interactive session containing
unsaved work. The review used subprocesses with core dumps disabled.

These scripts reproduce bugs rather than fixing them. Several assert the
observed incorrect behavior so the examples fail visibly once it changes.
