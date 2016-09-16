// [[Rcpp::depends(bigmemory)]]
#include <Rcpp.h>
#include <bigmemory/MatrixAccessor.hpp>

using namespace Rcpp;


/******************************************************************************/

template <typename T>
NumericVector R_squared(XPtr<BigMatrix> xpMat,
                        MatrixAccessor<T> macc,
                        const NumericVector& y,
                        const IntegerVector& rowInd,
                        const NumericVector& weights) {
  int n = rowInd.size();
  int m = xpMat->ncol();

  NumericVector res(m);

  double ySum = 0, yySum = 0, wSum = 0;
  double tmpY, tmpW;
  int ind;

  for (int i = 0; i < n; i++) {
    ind = rowInd[i] - 1;
    tmpW = weights[i];
    tmpY = y[ind];
    wSum += tmpW;
    ySum += tmpY * tmpW;
    yySum += tmpY * tmpY * tmpW;
  }
  double denoY = yySum - ySum * ySum / wSum;

  double xSum, xySum, xxSum;
  double tmp;
  double num, denoX;

  for (int j = 0; j < m; j++) {
    xSum = xySum = xxSum = 0;
    for (int i = 0; i < n; i++) {
      ind = rowInd[i] - 1;
      tmpW = weights[i];
      tmp = macc[j][ind];
      xSum += tmp * tmpW;
      xySum += tmp * y[ind] * tmpW;
      xxSum += tmp * tmp * tmpW;
    }
    num = xySum - xSum * ySum / wSum;
    denoX = xxSum - xSum * xSum / wSum;
    res[j] = num * num / (denoX * denoY);
  }

  return(res);
}

// Dispatch function for R_squared
// [[Rcpp::export]]
NumericVector R_squared(SEXP pBigMat,
                        const NumericVector& y,
                        const IntegerVector& rowInd,
                        const NumericVector& weights) {
  // First we have to tell Rcpp what class to use for big.matrix objects.
  // This object stores the attributes of the big.matrix object passed to it
  // by R.
  XPtr<BigMatrix> xpMat(pBigMat);

  // To access values in the big.matrix, we need to create a MatrixAccessor
  // object of the appropriate type. Note that in every case we are still
  // returning a NumericVector: this is because big.matrix objects only store
  // numeric values in R, even if their type is set to 'char'. The types
  // simply correspond to the number of bytes used for each element.
  switch(xpMat->matrix_type()) {
  case 1:
    return R_squared(xpMat, MatrixAccessor<char>(*xpMat),   y, rowInd, weights);
  case 2:
    return R_squared(xpMat, MatrixAccessor<short>(*xpMat),  y, rowInd, weights);
  case 4:
    return R_squared(xpMat, MatrixAccessor<int>(*xpMat),    y, rowInd, weights);
  case 6:
    return R_squared(xpMat, MatrixAccessor<float>(*xpMat),  y, rowInd, weights);
  case 8:
    return R_squared(xpMat, MatrixAccessor<double>(*xpMat), y, rowInd, weights);
  default:
    // This case should never be encountered unless the implementation of
    // big.matrix changes, but is necessary to implement shut up compiler
    // warnings.
    Function err("ERROR_TYPE");
    throw Rcpp::exception(as<const char*>(err()));
  }
}

/******************************************************************************/

template <typename T>
NumericMatrix betasRegLin(XPtr<BigMatrix> xpMat,
                          MatrixAccessor<T> macc,
                          const NumericVector& y,
                          const IntegerVector& rowInd,
                          const NumericVector& weights) {
  int n = rowInd.size();
  int m = xpMat->ncol();

  NumericMatrix res(2, m);

  double ySum = 0, wSum = 0;
  double tmpW;
  int ind;

  for (int i = 0; i < n; i++) {
    ind = rowInd[i] - 1;
    tmpW = weights[i];
    wSum += tmpW;
    ySum += y[ind] * tmpW;
  }

  double xSum, xySum, xxSum;
  double tmp, tmpB;
  double num, denoX;

  for (int j = 0; j < m; j++) {
    xSum = xySum = xxSum = 0;
    for (int i = 0; i < n; i++) {
      ind = rowInd[i] - 1;
      tmp = macc[j][ind];
      tmpW = weights[i];
      xSum += tmp * tmpW;
      xySum += tmp * y[ind] * tmpW;
      xxSum += tmp * tmp * tmpW;
    }
    num = xySum - xSum * ySum / wSum;
    denoX = xxSum - xSum * xSum / wSum;
    tmpB = num / denoX;
    res(1, j) = tmpB;
    res(0, j) = (ySum - tmpB * xSum) / wSum;
  }

  return(res);
}

// Dispatch function for betasRegLin
// [[Rcpp::export]]
NumericMatrix betasRegLin(SEXP pBigMat,
                          const NumericVector& y,
                          const IntegerVector& rowInd,
                          const NumericVector& weights) {
  // First we have to tell Rcpp what class to use for big.matrix objects.
  // This object stores the attributes of the big.matrix object passed to it
  // by R.
  XPtr<BigMatrix> xpMat(pBigMat);

  // To access values in the big.matrix, we need to create a MatrixAccessor
  // object of the appropriate type. Note that in every case we are still
  // returning a NumericVector: this is because big.matrix objects only store
  // numeric values in R, even if their type is set to 'char'. The types
  // simply correspond to the number of bytes used for each element.
  switch(xpMat->matrix_type()) {
  case 1:
    return betasRegLin(xpMat, MatrixAccessor<char>(*xpMat),
                       y, rowInd, weights);
  case 2:
    return betasRegLin(xpMat, MatrixAccessor<short>(*xpMat),
                       y, rowInd, weights);
  case 4:
    return betasRegLin(xpMat, MatrixAccessor<int>(*xpMat),
                       y, rowInd, weights);
  case 6:
    return betasRegLin(xpMat, MatrixAccessor<float>(*xpMat),
                       y, rowInd, weights);
  case 8:
    return betasRegLin(xpMat, MatrixAccessor<double>(*xpMat),
                       y, rowInd, weights);
  default:
    // This case should never be encountered unless the implementation of
    // big.matrix changes, but is necessary to implement shut up compiler
    // warnings.
    Function err("ERROR_TYPE");
    throw Rcpp::exception(as<const char*>(err()));
  }
}

/******************************************************************************/
