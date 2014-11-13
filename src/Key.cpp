// (title)
// (standard MIT disclaimer)  
//
// author: Svetoslav Tzvetkov
//         Massachusetts Institute of Technology
//         March 1998


#include <assert.h>
#include "Key.h"
#include "Invert.cpp"


// generate a random private basis
static void generate_private_basis(int dim,bigint_lattice_basis &R,
				   int range=4,int k=-1) {

  if (k<0)  k = range * (int)(1.5+sqrt((double)dim));
  bigint_matrix A(dim,dim);
  // start w/ random entries in (-range..range)
  A.randomize(2*range);
  // A -= range;
  for (int i=0; i<dim; i++) for (int j=0; j<dim; j++)  A.sto(i,j,A(i,j)-range);

  // add k*I
  if (k!=0)
    for (int i=0; i<dim; i++)  A.sto(i,i,k+A(i,i));
  R = A;

  // reduce the basis
  R.lll();
}

// generate the private and public bases
void private_key::generate(int dim, int lll, int inv) {
  int mix = 2;
  int i,j,k;
  bigint_lattice_basis R(dim,dim),B(dim,dim);
  bigint_matrix Tb(dim,dim);
  int cols[dim];
  bigint_vector x(dim,dim,FIXED);

  // generate the private basis first
  generate_private_basis(dim,R);

  // then mix it:

  // initialize T with I
  T.resize(dim,dim);
  for (i=0; i<dim; i++) for (j=0; j<dim; j++)  T.sto(i,j,(i==j));
  
  while (mix--) {  // perform mix mixing steps

    // process columns in random order
    for (i=0; i<dim; i++)  cols[i]=i;
    for (i=dim; i!=0; i--) {
      srand(time(NULL));
      k = rand()%i;  
      j = cols[k];  cols[k] = cols[i-1];  // j is the column to be changed
      for (k=0; k<dim; k++)
        if (k==j)
          x[k] = 1;
        else{
          srand(time(NULL));
          x[k] = ((rand()>>12)%7+4)/5-1;  // {-1,0,1} biased towards 0
        }

      // compute the new column
      T.sto_column_vector(T*x,dim,j);
    }
  }

  // compute the public basis and the transformation matrix
  B = R*T;
  if (lll) {  // if lll flag is set then reduce the public basis
    B.lll_trans(Tb);
    T *= Tb;
  }
  this->B = B;

  // compute the inverse of the private basis
  if (inv)
    invert_bigint_matrix2(R, Rinv);
  else
    invert_bigint_matrix(R,Rinv);

  // invert T
  // (since T is unimodular, so is its inverse;
  //  therefore, we store the result in T)
  bigfloat_matrix Ti;
  bigint t;
  if (inv)
    invert_bigint_matrix2(T, Ti);
  else
    invert_bigint_matrix(T, Ti);

  for (i=0; i<dim; i++)  for (j=0; j<dim; j++) {
    round(Ti(i,j)).bigintify(t);
    T.sto(i,j,t);
  }
}


// create a public key
void private_key::make_public(public_key &pubkey) {
  pubkey.B = B;
}

bigint_vector public_key::encrypt(bigint_vector v,bigint_vector e) {
  assert (B.get_no_of_rows() == v.get_size());

  return B*v+e;
}

// encrypt an array of bits:
//  if method==1 then the message is encode in the error vector
//  otherwise it's in the lattice point
bigint_vector public_key::encrypt_bitstr(int_vector b) {
  int dim = B.get_no_of_rows();
  assert(dim == b.get_size());
  bigint_vector v(dim,dim,FIXED),e(dim,dim,FIXED);
  
  for (int i=0; i<dim; i++)
    if (method==1) {
      v[i] = (rand()%256)-128;
      if (rand()&1)  e[i]=-2;  else  e[i]=3;
      if (b[i])  e[i]=-e[i];
    } else {
      // v[i] = (((rand()%256)>>1)<<1)-128;
      // if (b[i])  v[i]++;
      if (rand()&1)  e[i]=-3;  else  e[i]=3;
      v[i] = b[i];
    }
  return  encrypt(v,e);
}


bigint_vector private_key::decrypt(bigint_vector c) {
  int i;
  int dim = Rinv.get_no_of_rows();
  assert(dim == c.get_size());

  // convert to reals
  bigfloat_vector u(dim,dim,FIXED);
  for (i=0; i<dim; i++)  u[i] = c[i];

  // u = Rinv*c
  u = Rinv*u;

  // round the result
  for (i=0; i<dim; i++)
    u[i] = (u[i]>0) ? floor(0.5+u[i]) : ceil(-0.5+u[i]);
  
  // convert to integers
  bigint_vector v(dim,dim,FIXED);
  bigint k;
  for (i=0; i<dim; i++) {
    u[i].bigintify(k);
    v[i] = k;
  }
  
  // multiply by the transformation matrix ( Binv=T*Rinv )
  v = T*v;
  return v;
}

// decrypt an array of bits:
//  if method==1 then the entries are encoded in the error vector
//  otherwise they are in the lattice vector
int_vector private_key::decrypt_bitstr(bigint_vector c) {
  int dim = c.get_size();
  bigint_vector v = decrypt(c);
  bigint_vector e = c-B*v;
  int_vector b(dim,dim,FIXED);

  for (int i=0; i<dim; i++)
    if (method==1) {
      b[i] = (e[i]==2 || e[i]==-3);
    } else {
      v[i].intify(b[i]);
      // b[i] &= 1;
    }

  return b;
}

// predefined stream operators for private and public keys
// note: you can print a key in a plain format or in matlab's format
// but you can read only from plain format

istream & operator >> (istream & in, private_key & privkey) {
  int i,j,dim;

  in >> dim;  // read the dimension
  privkey.Rinv.resize(dim,dim);
  privkey.T.resize(dim,dim);
  privkey.B.resize(dim,dim);

  // read Rinv
  bigfloat x;
  for (i=0; i<dim; i++)  for (j=0; j<dim; j++) {
    in >> x;
    privkey.Rinv.sto(i,j,x);
  }

  // read T
  bigint n;
  for (i=0; i<dim; i++)  for (j=0; j<dim; j++) {
    in >> n;
    privkey.T.sto(i,j,n);
  }

  // read B
  for (i=0; i<dim; i++)  for (j=0; j<dim; j++) {
    in >> n;
    privkey.B.sto(i,j,n);
  }

  return in;
}

ostream & operator << (ostream & out, const private_key & privkey) {
  int i,j,dim = privkey.Rinv.get_no_of_rows();

  if (privkey.print_format==1 || privkey.print_format==2) {
    
    if (privkey.print_format==1)  // skip if printing out for matlab
      out << dim << endl;
    
    // write Rinv
    for (i=0; i<dim; i++) {
      for (j=0; j<dim; j++)
	out << ' ' << privkey.Rinv(i,j);
      out << endl;
    }
    
    // write T
    for (i=0; i<dim; i++) {
      for (j=0; j<dim; j++)
	out << ' ' << privkey.T(i,j);
      out << endl;
    }
    
    // write B
    for (i=0; i<dim; i++) {
      for (j=0; j<dim; j++)
	out << ' ' << privkey.B(i,j);
      out << endl;
    }
  }
  return out;
}

istream & operator >> (istream & in, public_key & pubkey) {
  int i,j,dim;

  in >> dim;  // read the dimension
  pubkey.B.resize(dim,dim);

  // read B
  bigint n;
  for (i=0; i<dim; i++)  for (j=0; j<dim; j++) {
    in >> n;
    pubkey.B.sto(i,j,n);
  }

  return in;
}
ostream & operator << (ostream & out, const public_key & pubkey) {
  int i,j,dim = pubkey.B.get_no_of_rows();

  if (pubkey.print_format==1 || pubkey.print_format==2) {

    if (pubkey.print_format==1)  // skip if printing out for matlab
      out << dim;
    
    // write B
    for (i=0; i<dim; i++) {
      for (j=0; j<dim; j++)
	out << ' ' << pubkey.B(i,j);
      out << endl;
    }
    
  }

  return out;
}










