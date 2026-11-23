> Historical planning sketch. The current working draft is in Extending_randomForestSRC_to_SupportLeft_Truncated_Survival_Data_Thesis_draft_for_discussion/main.tex. Earlier Overleaf folders and ZIPs predate the user revisions.

# Extending randomForestSRC to Support Left-Truncated Survival Data

Working title and initial draft. Language and university formatting are pending confirmation. The requested writing plugin has not yet been identified or used.

## Proposed structure

An abstract, six chapters, references, and technical appendices should be enough for an initial applied thesis. The implementation is the central contribution; the simulations assess its behavior.

1. **Introduction.** Explain delayed entry, why ignoring it matters, the project's objectives, and the scope of the contribution.
2. **Background and related work.** Introduce right censoring, left truncation, risk sets, Cox regression, and random survival forests. Review existing tree and forest approaches for delayed-entry data before making novelty claims.
3. **Methods and software implementation.** Define the observed data and assumptions, explain entry-aware log-rank splitting and terminal-node estimation, and describe the R-to-C implementation. Separate fitting, prediction, and evaluation. Document supported options and limitations.
4. **Simulation study.** Specify the six data-generating processes, entry and censoring mechanisms, model comparisons, evaluation targets, hyperparameters, replication counts, and Monte Carlo uncertainty.
5. **Results.** Present implementation checks and simulation results. Distinguish prediction-curve accuracy from discrimination. Label the existing 10-replication results as preliminary.
6. **Discussion and conclusions.** Explain what the extension contributes, its limitations, reproducibility, and remaining work toward an upstream submission.

**Appendices:** implementation map, detailed formulas, supplementary results, and reproducible execution instructions.

## Provisional abstract

Left truncation occurs when individuals enter observation after the origin of the survival time scale and are observed only if they remain event-free until entry. Survival methods must account for this delayed entry when constructing risk sets. This thesis develops an extension of the randomForestSRC R package for left-truncated, right-censored survival data. The extension introduces subject-specific entry times into the package's R and C interfaces and modifies the default log-rank splitting rule and terminal-node estimators to use entry-aware risk sets. The work also examines prediction evaluation under delayed entry and implements corresponding scoring tools. A simulation framework compares Cox regression and random survival forests fitted with and without knowledge of entry times across six data-generating processes. Initial runs contain ten replications per scenario and are treated as preliminary. The final analysis will assess survival-curve accuracy and discrimination, quantify Monte Carlo uncertainty, and document the scope and limitations of the software extension.

## 1. Introduction — opening draft

Survival analysis studies the time until an event of interest, such as death, disease recurrence, or equipment failure. In many studies, observation does not begin at the origin of the relevant time scale. An individual may enter a study several years after diagnosis, for example, while follow-up is measured from the date of diagnosis. Such an individual can appear in the study only if the event has not occurred before entry. This observation mechanism is called left truncation, or delayed entry.

Left truncation differs from right censoring. With right censoring, an individual is observed but the event time is known only to exceed the end of follow-up. With left truncation, individuals whose event precedes entry are absent from the observed sample. Accounting for delayed entry therefore requires attention both to how the sample is selected and to who is under observation at each event time.

Random survival forests combine survival trees to estimate outcomes from covariates. The original random survival forest formulation addresses right-censored outcomes [1]. Tree methods have also been developed specifically for left-truncated, right-censored data [2]. This thesis focuses on bringing delayed-entry support into the main fitting interface and computational core of the randomForestSRC package. Its contribution is an applied software extension and its evaluation; it does not claim to introduce the first tree-based method for left truncation.

The implementation centers on the risk set. At a given event time, an individual should contribute to the risk set only after entering observation and before leaving follow-up. The extension applies this requirement to the default log-rank splitting rule and to the terminal-node estimators used to construct predicted survival and cumulative hazard curves. An optional entry-time argument connects the user-facing R function to these calculations in C.

The empirical study asks how accounting for entry times changes model performance. Cox regression and random survival forests are each fitted twice to the same observed sample: once ignoring delayed entry and once using the observed entry times. The simulation scenarios include both correctly specified Cox settings and departures from its modeling assumptions. Evaluation distinguishes accuracy of predicted survival curves from the ability to rank individuals by risk. The current results are exploratory; final conclusions require a larger simulation run and uncertainty estimates.

The objectives are to implement delayed-entry support, verify the resulting calculations and backward compatibility, evaluate performance under controlled simulation conditions, and document the extension for a future upstream contribution. Upstream submission is a subsequent goal and is not presented as a completed outcome of this draft.

## Decisions and checks before expanding the draft

- Confirm the university template, required language, and expected length with the supervisor.
- Resolve the entry-time boundary convention consistently across formulas, code, and comparisons with Cox regression. Current documents alternate between strict and inclusive entry.
- State the assumptions needed for identification under truncation and censoring, including any conditional independence and support requirements.
- Define each evaluation target explicitly: population survival, survival conditional on entry, and the population over which a score is averaged are different choices.
- Review the Brier and AUC weighting assumptions before presenting their implementations as validated statistical estimators. Agreement with a programmed specification alone does not establish statistical validity.
- Distinguish external scoring of predictions from the error and variable importance computed internally by predict().
- Validate numerical calculations against independent references, in addition to checking that zero entry reproduces the package's ordinary fitting path.
- Report Monte Carlo uncertainty rather than interpreting small changes from ten replications as established findings.

## Initial references

1. Ishwaran, H., Kogalur, U. B., Blackstone, E. H., and Lauer, M. S. (2008). Random survival forests. *The Annals of Applied Statistics*, 2(3), 841–860. https://doi.org/10.1214/08-AOAS169. Author manuscript: https://arxiv.org/abs/0811.1645.
2. Fu, W., and Simonoff, J. S. (2017). Survival trees for left-truncated and right-censored data, with application to time-varying covariate data. *Biostatistics*, 18(2), 352–369. https://doi.org/10.1093/biostatistics/kxw047. Author manuscript: https://arxiv.org/abs/1606.03033.

This is a starting bibliography, not a completed literature review.
