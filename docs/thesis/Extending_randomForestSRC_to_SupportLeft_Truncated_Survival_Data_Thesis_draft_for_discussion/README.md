# Working discussion draft for Overleaf

This is the user-revised Overleaf export and the base for subsequent edits.
Keep the deliberate omissions and `TBD` sections unless asked to develop them.

The numerical basis is the completed **100 replications
per scenario (600 total)** at package commit 1802cc3, using the isolated run
simulations/reruns/2026-11-06-1802cc3/. Original saved ten-replication results
are not used by this draft.

## Read or upload

Upload the contents of this folder to Overleaf and select main.tex with
pdfLaTeX. The older `overleaf-*-draft.zip` archives do not contain these
user revisions. The project uses BibTeX, separate chapter files, generated numerical
tables, and two included PNG figures. It needs no external scripts, R
installation, or shell escape.

The author, supervisor, institution, and university formatting remain
unspecified. The literature review and full DGP/source descriptions are still
draft material; this is a discussion draft, not a completed thesis.

## What changed

- Replaced ten-replication preliminary numbers with all six completed
  100-replication experiments, including paired Monte Carlo standard errors.
- Added the four-metric summary and absolute concordance, Brier/CRPS, and AUC
  tables, with correct error scales and percentage-change interpretation.
- Updated the implementation and validation record, supported options,
  prediction/restoration/tuning behavior, and computational limitations.
- Made the inclusive-entry convention, returned survival-curve construction,
  Brier interpretation, and added AUC definition explicit.
- Preserved the mixed finding: curve accuracy improves across scenarios,
  while RSF concordance worsens in some scenarios.
- Distinguished source-population curve accuracy from entered-cohort scores
  and documented pooled-censoring and evaluation-sample limitations.

The project outline is the governing project specification. The expanded
Brier equation is our interpretation of its Section 9.2; the AUC estimator is
an additional project metric. Neither is described as an equation prescribed
by the outline. Survival predictions use the package's existing average of
tree-specific Kaplan--Meier curves; this aggregation choice is settled for
the current study. Calibration and covariate-stratum analyses remain
discussion topics.

The remaining Brier and AUC alignment questions use `\alignmentnote{...}`
and appear in red with the label "Hi, we need alignment:" in the PDF.
Factual source references use "project outline".

## Regenerate numerical tables

From the repository root, run:

    Rscript docs/thesis/generate_results_tables.R

This only reads the completed isolated result objects. It validates run sizes,
finite scalar scores, zero-entry checks, and matching package provenance, then
generates the tables and run metadata. It does not fit models or rerun
simulations. The previous generate_preliminary_table.R entry point now
forwards to this generator so it cannot silently restore old numbers.
The tables/preliminary-mad.tex filename is also a compatibility redirect.

The figures are copies of the MAD and concordance charts from the same
refreshed summary. Review numerical results, captions, and narrative together
if another run replaces this study.

## Validation and archives

The update is checked against the saved results, with input/figure paths,
cross-references, bibliography keys, and ZIP contents checked locally.
No TeX compiler is installed here, so local PDF compilation was not performed;
compile the uploaded project in Overleaf before circulating a PDF.

The parent directory's `overleaf/` folder and `overleaf-*-draft.zip` files
are earlier versions. The numerical generator targets this working folder;
it does not restore removed chapter text. No Overleaf upload, Git commit,
or GitHub push is performed by these local edits.
