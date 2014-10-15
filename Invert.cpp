
// invert a integer matrix using gaussian elimination
static void invert_bigint_matrix(const bigint_matrix &A_, bigfloat_matrix &B_) {
#define A(i,j) ((A[(i)])[(j)])
#define B(i,j) ((B[(i)])[(j)])
  int N,i,j,row;
  bigfloat dummy,lambda;

  assert(A_.get_no_of_rows() == A_.get_no_of_columns());
  N = A_.get_no_of_rows();
  B_.resize(N,N);

  bigfloat **A, **B;
  A = B_.get_data();  // easy way to allocate memory ;)
  B = B_.get_data();  //

  for (i=0; i<N; i++)  for (j=0; j<N; j++)  // copy A
    A(i,j) = (bigfloat)A_(i,j);

  for (i=0; i<N; i++)  for (j=0; j<N; j++)  // initialize B with I
    B(i,j) = (bigfloat)(i==j);

  for (row=0;row<N;row++) {
    for (i=row; A(i,row).is_approx_zero() && i<N; i++); // find the first row
    assert(i!=N);                                       // with a nonzero entry
    if (i!=row) {                                       // in row-th column
      for (j=0; j<N; j++) {
	dummy=A(row,j); A(row,j)=A(i,j); A(i,j)=dummy; // exchange with
	dummy=B(row,j); B(row,j)=B(i,j); B(i,j)=dummy; // row-th row
      }
    }
    lambda = A(row,row);
    for (j=0; j<N; j++) {
      A(row,j) = A(row,j)/lambda;  // normalize the row
      B(row,j) = B(row,j)/lambda;
    }
    for (i=row+1; i<N; i++) {  // subtract from other rows
      lambda = A(i,row);
      for (j=0; j<N; j++) {
	A(i,j) -= A(row,j)*lambda;
	B(i,j) -= B(row,j)*lambda;
      }
    }
  }

  for (row=N-1; row>0; row--)  // diagonalize A
    for (i=0; i<row; i++) {
      lambda = A(i,row);
      for (j=0; j<N; j++) {
	A(i,j) -= A(row,j)*lambda;
	B(i,j) -= B(row,j)*lambda;
      }
    }

  
  B_.set_data((const bigfloat **)B,N,N);
#undef B
#undef A
}


// use LiDIA invert
static void invert_bigint_matrix2(const bigint_matrix &A_,bigfloat_matrix &B_) {
  int N;
  assert(A_.get_no_of_rows() == A_.get_no_of_columns());
  N = A_.get_no_of_rows();
  
  bigfloat_matrix A(N,N);
  for (int i=0; i<N; i++)
    for (int j=0; j<N; j++)
      A.sto(i,j,A_(i,j));
  A.invert(B_);
}

