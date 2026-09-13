# Brier Score with Left Truncation

The **Brier Score** evaluates the overall performance and calibration of a survival model at a specific time point $t$. While metrics like the $C$-index evaluate relative ranking, the Brier Score measures the accuracy of individual predicted survival probabilities against observed outcomes.

---

## 1. Standard Brier Score (Right-Censored Data)

At a given evaluation time $t$, the Brier Score calculates the Mean Squared Error (MSE) between actual survival status $Y_i(t)$ and the model's predicted survival probability $\hat{S}(t \mid X_i) = P(T_i > t \mid X_i)$.

### Observed Survival Status
$$Y_i(t) = \mathbb{I}(T_i > t)$$

- **$Y_i(t) = 1$**: Subject survived past time $t$.
- **$Y_i(t) = 0$**: Subject experienced the event on or before time $t$.

### Handling Right Censoring (IPCW)
Because right-censoring prevents observing $Y_i(t)$ for subjects censored before time $t$, standard implementations apply **Inverse Probability of Censoring Weighting (IPCW)** using the Kaplan-Meier estimate of the censoring distribution $\hat{G}_C(t) = P(C > t)$:

$$\text{BS}(t) = \frac{1}{N} \sum_{i=1}^N \left[ \frac{(0 - \hat{S}(t \mid X_i))^2 \cdot \mathbb{I}(T_i \le t, \delta_i = 1)}{\hat{G}_C(T_i^-)} + \frac{(1 - \hat{S}(t \mid X_i))^2 \cdot \mathbb{I}(T_i > t)}{\hat{G}_C(t)} \right]$$

- **Interpretation:** $0$ indicates perfect prediction accuracy; $0.25$ corresponds to an uninformative baseline model (predicting 50% probability for everyone).

---

## 2. Adapting the Brier Score to Left Truncation

Under **left truncation** (late entry $T_{L, i} > 0$), two crucial adjustments are necessary:

### Adjustment 1: Dynamic Risk Set at Time $t$
Subjects who enter the study after evaluation time $t$ ($T_{L, i} > t$) were not observable at time $t$. They must be excluded from the evaluation set $\mathcal{R}_L(t)$:

$$\mathcal{R}_L(t) = \{i : T_{L, i} \le t\}$$

### Adjustment 2: Conditional Survival Predictions
Subjects observed in the study have already survived up to their entry time $T_{L, i}$. The model must predict **conditional survival probability** given $T_i > T_{L, i}$:

$$\hat{S}_{\text{cond}}(t \mid X_i, T_i > T_{L, i}) = \frac{\hat{S}(t \mid X_i)}{\hat{S}(T_{L, i} \mid X_i)}$$

---

## 3. Left-Truncated Brier Score Formula ($BS_{\text{LT}}$)

To adjust for selection bias and delayed entry, weights combine censoring weights $G_C(t)$ with truncation weights $G_L(t) = P(T_L \le t)$:

$$w_i(t) = \begin{cases} 
\frac{1}{G_C(T_i^-) \cdot G_L(T_i^-)} & \text{if } T_i \le t, \delta_i = 1 \\[8pt]
\frac{1}{G_C(t) \cdot G_L(t)} & \text{if } T_i > t 
\end{cases}$$

$$\text{BS}_{\text{LT}}(t) = \frac{1}{|\mathcal{R}_L(t)|} \sum_{i \in \mathcal{R}_L(t)} w_i(t) \cdot \left[ Y_i(t) - \hat{S}_{\text{cond}}(t \mid X_i, T_i > T_{L, i}) \right]^2$$

---

## 4. Summary

The Brier Score for left truncation evaluates mean squared prediction error at a fixed time $t$ by:
1. Restricting evaluation to subjects who entered the study on or before $t$ ($T_{L, i} \le t$).
2. Conditioning predicted survival probabilities on survival up to delayed entry $T_{L, i}$.
3. Weighting observations via joint IPCW / Inverse Probability of Truncation Weighting (IPTW).
