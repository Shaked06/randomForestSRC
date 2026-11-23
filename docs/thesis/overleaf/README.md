# Updated thesis draft for Overleaf

This version has been superseded by the user-revised draft in
`../Extending_randomForestSRC_to_SupportLeft_Truncated_Survival_Data_Thesis_draft_for_discussion/`.
Use that folder for subsequent edits. The instructions below describe this
earlier version; the table generator now targets the user-revised draft.

Updated 7 November 2026. The numerical basis is the completed **100 replications
per scenario (600 total)** at package commit 1802cc3, using the isolated run
simulations/reruns/2026-11-06-1802cc3/. Original saved ten-replication results
are not used by this draft.

Bibliography corrected 9 November 2026: the Brier methods section now cites
the JMbayes2 dynamic-scoring documentation and Fu--Simonoff supplement
recorded in the development notes. Their roles and differences from our
entry-conditioned estimator are explicit. Numerical results are unchanged.

## Read or upload

Use **overleaf-updated-draft.zip** in the parent directory as the current
upload package. In Overleaf, upload it as a project and select main.tex with
pdfLaTeX. The project uses BibTeX, separate chapter files, generated numerical
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

The professor's outline is the governing project specification. The expanded
Brier equation is our interpretation of its Section 9.2; the AUC estimator is
an additional project metric. Neither is described as an equation prescribed
by the outline. The survival aggregation choice and remaining calibration/
covariate-stratum analyses are listed for supervisor review.

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

The parent directory's overleaf-initial-draft.zip is the historical initial
archive. The current archive is overleaf-updated-draft.zip. No Overleaf upload,
Git commit, or GitHub push is performed by this update.
