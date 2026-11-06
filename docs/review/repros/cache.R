## Rscript docs/review/repros/cache.R, from the repository root.
## Demonstrates the stage1 cache-key omission without changing package files.
## A temporary in-memory replacement stands in for a package reinstall.
suppressPackageStartupMessages(library(randomForestSRC))
suppressPackageStartupMessages(library(knitr))
local({
  work <- tempfile("review-knitr-")
  dir.create(work)
  original <- randomForestSRC::get.cindex
  on.exit(assignInNamespace("get.cindex", original, ns = "randomForestSRC"))
  paths <- normalizePath(c(
    "simulations/01_cox_random_forerst_lt/setup.rds",
    "simulations/01_cox_random_forerst_lt/dgp.R",
    "simulations/_common/sim_engine.R"))
  text <- paste0(
    "```{r loop, cache=TRUE, cache.extra=list(10L, 200L, tools::md5sum(c(",
    paste(sprintf('"%s"', paths), collapse = ","),
    "))), cache.path=\"", work, "/cache/\"}\n",
    "res <- randomForestSRC::get.cindex(1:3,c(1,1,0),3:1)\n",
    "print(res)\n```\n")
  assignInNamespace("get.cindex", function(...) .111, ns = "randomForestSRC")
  first <- knit(text = text, output = file.path(work, "first.md"),
                quiet = TRUE, envir = new.env(parent = globalenv()))
  assignInNamespace("get.cindex", function(...) .999, ns = "randomForestSRC")
  second <- knit(text = text, output = file.path(work, "second.md"),
                 quiet = TRUE, envir = new.env(parent = globalenv()))
  first.text <- paste(readLines(first), collapse = " ")
  second.text <- paste(readLines(second), collapse = " ")
  stopifnot(grepl("0.111", first.text, fixed = TRUE),
            grepl("0.111", second.text, fixed = TRUE),
            randomForestSRC::get.cindex(1:3, c(1, 1, 0), 3:1) == .999)
  cat("BUG stale cache: package function changed from 0.111 to 0.999,",
      "but the second knit still reports 0.111.\n")
})
