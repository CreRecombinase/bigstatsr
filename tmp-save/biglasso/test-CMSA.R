################################################################################

context("CMSA")

ALL.METHODS <- eval(formals("big_CMSA")$method)[1]
ALL.SP_FUN <- sapply(paste0("big_sp", c("LinReg", "LogReg")), get) # , "SVM"

# Simulating some data
N <- 911 # Some issues for small sample sizes
M <- 230
x <- matrix(rnorm(N * M, mean = 100, sd = 5), N)
y <- sample(0:1, size = N, replace = TRUE)
y.factor <- factor(y, levels = c(1, 0))

covar0 <- matrix(rnorm(N * 3), N)
lcovar <- list(NULL, covar0)

################################################################################

test_that("correlation between predictors", {
  for (t in sample(TEST.TYPES, size = 2)) {
    X <- `if`(t == "raw", asFBMcode(x), big_copy(x, type = t))

    for (f in ALL.SP_FUN) {
      covar <- sample(lcovar)[[1]]
      alpha <- runif(1)
      lambda.min <- runif(1, min = 0.01, max = 0.5)
      meth <- sample(ALL.METHODS, size = 1)

      mod.bigstatsr <- f(X, y, covar.train = covar, alpha = alpha,
                         lambda.min = lambda.min)
      beta.lol <- get_beta(mod.bigstatsr$beta[, 20:60], meth)
      pred.lol <- predict(mod.bigstatsr, X = X, covar.row = covar)

      beta.cmsa <- big_CMSA(big_spLogReg, feval = AUC, X = X,
                            y.train = y, covar.train = covar,
                            method = meth, ncores = test_cores())
      pred.cmsa <- predict(beta.cmsa, X = X, covar.row = covar)

      cor.pval <- cor.test(beta.lol, beta.cmsa)$p.value
      printf("(%.0e)", cor.pval)
      expect_lt(cor.pval, 0.01)

      cor.pval2 <- cor.test(get_beta(pred.lol[, 20:60], meth),
                            pred.cmsa)$p.value
      printf("(%.0e)", cor.pval2)
      expect_lt(cor.pval2, 0.01)
    }
  }
})

################################################################################
