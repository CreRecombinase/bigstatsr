/******************************************************************************/

#include "bigstatsr.h"

/******************************************************************************/

// [[Rcpp::export]]
NumericMatrix linRegPcadapt_cpp(const S4& BM,
                                arma::mat& U,
                                const IntegerVector& rowInd) {



  XPtr<BigMatrix> xpMat = BM.slot("address");
  int m = xpMat->ncol();
  RawSubMatAcc macc(*xpMat, rowInd-1, seq_len(m)-1, BM.slot("code"));
  int n = macc.nrow();
  int K = U.n_cols;
  myassert((int)U.n_rows == n, ERROR_DIM);

  arma::mat res(K, m);
  arma::vec y(n), betas(K), eps(n);
  double sum_y;
  int i, j;

  for (j = 0; j < m; j++) {
    sum_y = 0;
    for (i = 0; i < n; i++) { // j-th SNP (centered)
      y[i] = macc(i, j);
      sum_y += y[i];
    }
    y -= sum_y / n;

    betas = U.t() * y;
    eps = y - U * betas;
    res.col(j) = betas / sqrt(dot(eps, eps) / (n - K));
  }

  return as<NumericMatrix>(wrap(res.t()));
}

/******************************************************************************/
