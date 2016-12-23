// [[Rcpp::depends(bigmemory, BH, RcppArmadillo)]]
#include <RcppArmadillo.h> // Sys.setenv("PKG_LIBS" = "-llapack")
#include <bigmemory/MatrixAccessor.hpp>

using namespace Rcpp;


/******************************************************************************/

arma::mat getXtW(const arma::mat& covar, const arma::vec& w) {
  return trans(covar.each_col() % w);
}

/******************************************************************************/

template <typename T>
ListOf<SEXP> IRLS(XPtr<BigMatrix> xpMat,
                  MatrixAccessor<T> macc,
                  arma::mat& covar,
                  const arma::vec& y,
                  const arma::vec& z0,
                  const arma::vec& w0,
                  const IntegerVector& rowInd,
                  double tol,
                  int maxiter) {
  int n = rowInd.size();
  int m = xpMat->ncol();
  arma::mat tcovar, tmp;
  arma::vec p, w, z, betas_old, betas_new, Xb;
  double diff;
  int c;

  // indices begin at 1 in R and 0 in C++
  IntegerVector trains = rowInd - 1;

  NumericVector res(m);
  NumericVector var(m);
  IntegerVector conv(m);

  for (int j = 0; j < m; j++) {
    for (int i = 0; i < n; i++) {
      covar(i, 0) = macc[j][trains[i]];
    }
    z = z0;
    w = w0;
    tcovar = getXtW(covar, w);
    betas_new = solve(tcovar * covar, tcovar * z);
    c = 1;

    do {
      c++;
      betas_old = betas_new;

      Xb = covar * betas_old;
      p = 1 / (1 + exp(-Xb));
      w = p % (1 - p);
      z = Xb + (y - p) / w;

      tcovar = getXtW(covar, w);
      betas_new = solve(tcovar * covar, tcovar * z);

      diff = 2 * abs(betas_old(0) - betas_new(0))
        / (abs(betas_old(0)) + abs(betas_new(0)));
    } while (diff > tol && c < maxiter);

    res[j] = betas_new(0);
    tmp = inv(tcovar * covar);
    var[j] = tmp(0, 0);
    conv[j] = c;
  }

  return(List::create(_["betas"] = res,
                      _["std"] = sqrt(var),
                      _["conv"] = conv));
}

/******************************************************************************/

// Dispatch function for IRLS
// [[Rcpp::export]]
ListOf<SEXP> IRLS(SEXP pBigMat,
                  arma::mat& covar,
                  const arma::vec& y,
                  const arma::vec& z0,
                  const arma::vec& w0,
                  const IntegerVector& rowInd,
                  double tol,
                  int maxiter) {
  XPtr<BigMatrix> xpMat(pBigMat);

  switch(xpMat->matrix_type()) {
  case 1:
    return IRLS(xpMat, MatrixAccessor<char>(*xpMat),   covar, y,
                z0, w0, rowInd, tol, maxiter);
  case 2:
    return IRLS(xpMat, MatrixAccessor<short>(*xpMat),  covar, y,
                z0, w0, rowInd, tol, maxiter);
  case 4:
    return IRLS(xpMat, MatrixAccessor<int>(*xpMat),    covar, y,
                z0, w0, rowInd, tol, maxiter);
  case 6:
    return IRLS(xpMat, MatrixAccessor<float>(*xpMat),  covar, y,
                z0, w0, rowInd, tol, maxiter);
  case 8:
    return IRLS(xpMat, MatrixAccessor<double>(*xpMat), covar, y,
                z0, w0, rowInd, tol, maxiter);
  default:
    throw Rcpp::exception("unknown type detected for big.matrix object!");
  }
}

/******************************************************************************/
