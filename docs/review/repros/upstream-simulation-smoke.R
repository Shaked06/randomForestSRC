## Integration audit for the pinned versions in UPSTREAM_SYNC_REVIEW.md.
suppressPackageStartupMessages(library(randomForestSRC))
options(rf.cores=1,mc.cores=1)
args <- commandArgs(TRUE)
repo <- args[1]
workspace <- args[2]
output <- args[3]
folders <- basename(Sys.glob(file.path(workspace,'simulations','*','setup.rds')))
folders <- basename(dirname(Sys.glob(file.path(workspace,'simulations','*','setup.rds'))))
results <- list()
for (folder in folders) {
  e <- new.env(parent=globalenv())
  sys.source(file.path(repo,'simulations','_common','sim_engine.R'),e)
  sys.source(file.path(repo,'simulations',folder,'dgp.R'),e)
  setup <- readRDS(file.path(workspace,'simulations',folder,'setup.rds'))
  setup$ltrc$n_ltrc <- 200L
  setup$ltrc$n_pool <- 5000L
  setup$rsf_hp$ntree <- 25L
  setup$rsf_hp$nodesize <- 5L
  result <- e$run_replication(1L,setup,e$sim,m_test=30L,verify_noop=TRUE)
  stopifnot(result$noop['rsf']==0,result$noop['cox'] < 1e-10,
            length(result$brier)==8L,length(result$auc)==8L,
            all(is.finite(result$cindex)),all(is.finite(result$brier)),
            all(is.finite(result$auc)))
  results[[folder]] <- data.frame(scenario=folder,cox_zero_gap=result$noop['cox'],
                  rsf_zero_gap=result$noop['rsf'],
                  brier_min=min(result$brier),brier_max=max(result$brier),
                  auc_error_min=min(result$auc),auc_error_max=max(result$auc))
  cat(folder,'PASS\n')
}
write.csv(do.call(rbind,results),output,row.names=FALSE)
