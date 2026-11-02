# Brier score: interpretation and fix 5

## Governing specification supplied by the user

The user identified `docs/thesis/left_truncated_RSF_student_project.pdf` as
the professor's source of truth. Section 9.2, page 7, specifies squared error
using `S_hat(t | X_i)`, restriction to subjects with `L_i <= t`, and censoring
weights estimated using delayed-entry risk sets. It also calls for conditioning
on being under observation at the relevant time.

This supersedes the earlier request to choose a target below. The implemented
interpretation is prediction after entry for enrolled subjects (Option A):
condition both predictions and censoring probabilities on reaching entry,
estimate censoring with delayed-entry risk sets, and omit entry-CDF weights.
Inclusive entry uses left limits at entry. The full formula, assumptions,
and API limits are documented in `docs/brier_score_left_truncation.md`.

The outline does not explicitly prescribe an automatic survival ratio, but
its observation-conditioning language does not rule it out. The ratio is
our derivation of that language, not a verbatim equation from the PDF.
This interpretation needs scientific review with the professor. Implementation
and regression tests are complete; the user approved the local fix-5 commit.

## Earlier analysis

The current implementation conditions each survival curve on its subject's
entry time, averages over subjects entered by the evaluation time, and also
applies inverse truncation weights. Those operations mix targets. Choosing
the target is a scientific decision; a code change should state it explicitly.

## Option A: prediction after entry for enrolled subjects

Example: a person enters at year 2, and we evaluate survival to year 5 given
that the person survived to entry. The relevant predicted probability is
`S(5|X) / S(2|X)` under the corresponding conditional independence assumptions.

At time t, score subjects who have entered by t. With fully observed event
times, the empirical score is their ordinary average squared prediction
error. Correctly specified censoring weights are needed when status at t is
unobserved because follow-up ended earlier. Population truncation weights
are not automatically part of this entered-cohort target.

This matches the conditioning in the project's existing Brier notes, but it
differs from the source-population target used in the truth-based MAD study.

## Option B: prediction from time zero for the source population

Example: evaluate survival to year 5 from diagnosis, including the population
whose earlier failures prevented enrollment. The predicted probability is
`S(5|X)`.

Scoring on a truncated sample requires a justified correction for selection,
censoring, and normalization, together with adequate support and assumptions.
It cannot be obtained merely by adding weights to the entry-conditioned
predictions in Option A. Bug 6 (estimating the source entry distribution from
selected observations) is an additional issue for this route.

This aligns the prediction target with the simulation's population MAD target.
The source population's covariate distribution must also be distinguished from
the observed cohort's distribution when defining a population-average score.

## Self-contained counterexample

Let independent source entry and failure times be uniform on (0,1), retaining
only L<T. There is no censoring. Fix entry l=.25 and evaluation time t=.5.
True entry-conditional survival is p=(1-t)/(1-l)=2/3.

The observed entry distribution is Fobs(u)=2u-u^2. For a supplied conditional
prediction q, the implemented limiting weighted squared loss at this entry is

    A*q^2 + B*(1-q)^2,

where

    A = integral_l^t 1/Fobs(u) du / (1-l),
    B = p/Fobs(t).

Its minimizer is B/(A+B), approximately .6115, instead of 2/3. Even replacing
Fobs with the correct source CDF F(u)=u gives a minimizer near .5906.
Thus fixing bug 6 alone does not repair the stated entry-conditional Brier
interpretation. Ordinary squared loss, by comparison, is minimized at p.

This does not rule out a deliberately reweighted target; it shows that such a
target would differ from the prediction probability currently claimed.

## Censoring still needs explicit treatment

Either route must define which censoring probability is used in its weights.
For an entry-conditioned score, it must describe follow-up availability for
the enrolled subject, conditional on the information required by the sampling
mechanism. A pooled right-censored Kaplan–Meier estimate does not automatically
provide that quantity under every delayed-entry design. In particular, the
simulation scenarios use several different residual censoring mechanisms.

## Primary references consulted

- Fu and Simonoff, supplemental material, Section 4.1:
  https://pages.stern.nyu.edu/~jsimonof/survivaltree/supplemental.pdf.
  Their illustrated evaluation uses uncensored test event times; it should
  not be read as validation of the project's current three-adjustment formula.
- JMbayes2 author-maintained documentation, dynamic Brier scoring:
  https://drizopoulos.r-universe.dev/JMbayes2/doc/manual.html.
  Its prediction setup explicitly distinguishes a starting time and a later
  prediction horizon.

The counterexample motivates removing the extra entry weights. It does not
establish a source-population Brier estimator, nor does it validate every
censoring model. The pooled censoring option assumes a common hazard; the
forest option conditions on covariates and entry, with correctness depending
on that model. These assumptions are explicit in the implementation notes.
