
big_read <- function(file,
                     sep,
                     header,
                     confirmed = FALSE,
                     verbose = TRUE,
                     ind.skip = integer(0),
                     ind.meta = integer(0),
                     nlines = NULL,
                     nlines.block = NULL,
                     fun.con = function(f) file(f, open = "rt"),
                     type = NULL,
                     ...) {

  message3 <- function(...) {
    if (verbose) bigstatsr:::message2(...)
  }

  # Get #lines of the file
  if (is.null(nlines)) {
    nlines <- nlines_(file)
    # message3("%s lines detected.", nlines)
  }
  n <- nlines - header

  # Size of the blocks
  if (is.null(nlines.block)) {
    size.max <- getOption("bigstatsr.block.sizeGB") * 1024^3
    nlines.block <- min(max(1, floor(size.max / file.size(file) * n)), n)
    # message3("blocks of size used")
  }

  # Guess from data.table::fread()
  top_auto <- data.table::fread(file, nrows = nlines.block, data.table = FALSE)

  # Verify sep & header
  top_guided <- data.table::fread(file, nrows = nlines.block, data.table = FALSE, sep = sep, header = header)
  if (!identical(top_auto, top_guided)) {
    stop("There is a problem with either 'sep' or 'header'.")
  }

  # Meta -> df  &  numeric -> FBM
  coltypes <- unname(sapply(top_guided, typeof))
  coltypes.num <- bigstatsr:::ALL.TYPES[coltypes]
  is.meta <- is.na(coltypes.num)
  is.meta[ind.meta] <- TRUE
  is.num <- !is.meta
  is.meta[ind.skip] <- FALSE
  is.num[ind.skip] <- FALSE
  ind.meta.all <- which(is.meta)
  meta_diff <- setdiff(ind.meta.all, ind.meta)
  if (length(meta_diff) > 0) {
    message3(sprintf("Will add %s more columns to meta information.", length(meta_diff)))
  }

  p <- sum(is.num)
  colnames <- `if`(header, names(top_guided)[is.num], NULL)
  stopifnot((p + length(ind.meta.all) + length(ind.skip)) == ncol(top_guided))

  # Prepare the resulting Filebacked Big Matrix
  if (is.null(type)) {
    type <- `if`(any(coltypes[is.num] == "double"), "double", "integer")
  }
  message3("Will create a %s x %s FBM of type '%s'.", n, p, type)
  message3("Will create a %s x %s df of meta information.", n, length(ind.meta.all))

  if (!confirmed && interactive()) {
    readline(prompt = "Press [enter] to continue or [esc] to exit")
  }

  res <- FBM(nrow = n, ncol = p, type = type, init = NULL, ...)

  # Open connexion
  con <- fun.con(file)
  on.exit(close(con), add = TRUE)

  # Column types
  coltypes.meta <- names(top_guided)[is.meta]
  colclasses <- as.list(coltypes)
  for (i in ind.skip) colclasses[i] <- list(NULL)
  # Types after skipping some column
  no.skip <- !(cols_along(top_guided) %in% ind.skip)
  is.meta <- is.meta[no.skip]
  is.num <- is.num[no.skip]
  stopifnot(all( (is.meta + is.num) == 1 ))

  meta_df <- big_apply(res, a.FUN = function(X, ind) {
    df_part <- read.csv(
      con, sep = sep, header = header && (ind[1] == 1), nrows = length(ind),
      colClasses = colclasses, stringsAsFactors = FALSE)
    X[ind, ] <- as.matrix(df_part[is.num])
    stats::setNames(df_part[is.meta], coltypes.meta)
  }, a.combine = "rbind", ind = seq_len(n), block.size = nlines.block)

  # Returns
  list(FBM = res, colnames = colnames, meta = meta_df)
}

tmp <- cbind.data.frame(iris, matrix(round(rnorm(150 * 500)), 150))
csv <- "tmp-data/fake.csv"
sep <- "|"
header <- TRUE
data.table::fwrite(tmp, csv, quote = FALSE, col.names = header, sep = sep)
test <- big_read(csv, sep = sep, header = header, ind.meta = 1:5, ind.skip = 6:50, confirmed = TRUE, nlines.block = 40)
typeof(test$FBM)
dim(test$FBM)
str(test$meta)
