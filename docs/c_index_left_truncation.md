# Harrell's Concordance Index ($C$-Index) with Left Truncation

The **Concordance Index ($C$-index)** measures the discrimination power of a survival or time-to-event model. Specifically, it assesses how well the model predicts the relative ordering of event times for pairs of subjects.

---

## 1. Standard $C$-Index (Right-Censored Data)

In standard survival analysis, the $C$-index represents the probability that for a randomly selected pair of subjects $(i, j)$, the subject who experiences the event first had a higher predicted risk score.

- $C = 0.5$: Random ranking (no predictive power).
- $C = 1.0$: Perfect ordering of subject risk.

### Comparable Pairs
Due to right-censoring, not all subject pairs are comparable. A pair $(i, j)$ is **comparable** if we can definitively determine who experienced the event first:
1. Subject $i$ experiences an event at time $T_i$ ($\delta_i = 1$).
2. Subject $j$ survives longer than subject $i$ ($T_j > T_i$), regardless of whether subject $j$ is censored later.

### Mathematical Definition
Let $\eta_i$ denote the predicted risk score for subject $i$ (where higher $\eta$ implies shorter predicted survival / higher risk):

$$\hat{C} = \frac{\sum_{i, j} \mathbb{I}(\delta_i = 1) \cdot \mathbb{I}(T_i < T_j) \cdot \left[ \mathbb{I}(\eta_i > \eta_j) + 0.5 \cdot \mathbb{I}(\eta_i = \eta_j) \right]}{\sum_{i, j} \mathbb{I}(\delta_i = 1) \cdot \mathbb{I}(T_i < T_j)}$$

---

## 2. Adapting the $C$-Index to Left Truncation

**Left truncation** (late entry) occurs when subjects enter the study at delayed entry times $T_{L, i} > 0$ and are only observed if they remain event-free until $T_{L, i}$.

### The Issue with Naive Pairs
If subject $j$ enters the study at $T_{L, j}$, subject $j$ was **not under observation** before $T_{L, j}$. Comparing subject $j$ to subject $i$ when $T_i < T_{L, j}$ introduces immortal-time and selection bias, as subject $j$ could not have experienced an observed event at time $T_i$.

### Risk Set Filtering
To adjust for left truncation, a pair $(i, j)$ is defined as **comparable** at event time $T_i$ if and only if subject $j$ was actively under observation when subject $i$ experienced the event:

1. Subject $i$ experiences the event at time $T_i$ ($\delta_i = 1$).
2. Subject $j$ survived past $i$'s event time ($T_j > T_i$).
3. **Subject $j$ entered the study before or at $i$'s event time ($T_{L, j} \le T_i$).**

### Adjusted Formula ($C_{\text{LT}}$)

$$C_{\text{LT}} = \frac{\sum_{i, j} \mathbb{I}(\delta_i = 1) \cdot \mathbb{I}(T_i < T_j) \cdot \mathbb{I}(T_{L, j} \le T_i) \cdot \left[ \mathbb{I}(\eta_i > \eta_j) + 0.5 \cdot \mathbb{I}(\eta_i = \eta_j) \right]}{\sum_{i, j} \mathbb{I}(\delta_i = 1) \cdot \mathbb{I}(T_i < T_j) \cdot \mathbb{I}(T_{L, j} \le T_i)}$$

---

## 3. Inverse Probability of Truncation Weighting (IPTW)

To correct for temporal sampling biases induced by the truncation mechanism across different follow-up windows, inverse probability weighting can be applied using the estimated truncation distribution $\hat{G}(t) = P(T_L \le t)$:

$$w_i = \frac{1}{\hat{G}(T_i)}$$

Each valid pair $(i, j)$ is weighted by $w_i \cdot w_j$ in both the numerator and denominator to ensure unbiased population estimates.

---

## 4. Implementation Example (R)

```r
c_index_left_truncated <- function(start, stop, status, risk_score) {
  n <- length(stop)
  concordant <- 0
  comparable <- 0
  
  for (i in 1:n) {
    if (status[i] == 1) { # Subject i had the event
      t_event <- stop[i]
      
      for (j in 1:n) {
        if (i != j) {
          # Subject j must enter before i's event AND survive past i's event
          if (start[j] <= t_event && stop[j] > t_event) {
            comparable <- comparable + 1
            
            if (risk_score[i] > risk_score[j]) {
              concordant <- concordant + 1
            } else if (risk_score[i] == risk_score[j]) {
              concordant <- concordant + 0.5
            }
          }
        }
      }
    }
  }
  
  return(concordant / comparable)
}
```
