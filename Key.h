// (title)
// (standard MIT disclaimer)  
//
// author: Svetoslav Tzvetkov
//         Massachusetts Institute of Technology
//         March 1998

#ifndef  __Key_h__
#define  __Key_h__

#include "LiDIA/bigint_matrix.h"
#include "LiDIA/bigfloat_matrix.h"
#include "LiDIA/lattices/bi_lattice_basis.h"
#include <iostream>

using namespace std;
using namespace LiDIA;

typedef math_vector<int> int_vector;
typedef math_vector<bigint> bigint_vector;
typedef math_vector<bigfloat> bigfloat_vector;

class private_key;
class public_key;

istream & operator >> (istream & in, private_key & privkey);
ostream & operator << (ostream & out, const private_key & privkey);

istream & operator >> (istream & in, public_key & pubkey);
ostream & operator << (ostream & out, const public_key & pubkey);


class private_key {
private:
  bigfloat_matrix Rinv;
  bigint_matrix T;
  bigint_matrix B;
  int method;
  int print_format;

public:
  // constructor
  private_key() { method=1; print_format=1; }

  // encryption method: 1-message in error vector, 2-message in lattice point
  int get_method(void) { return method; }
  void set_method(int m) { method=m; }

  // print format: 1-plain, 2-matlab
  int get_print_format(void) { return print_format; }
  void set_print_format(int f) { print_format=f; }

  // generation functions
  void generate(int dim, int lll=0, int inv=0);
  void make_public(public_key &pubkey);

  // get the dimention of the basis
  int get_dim(void) { return Rinv.get_no_of_rows(); }

  // decryption functions
  bigint_vector decrypt(bigint_vector c);
  int_vector decrypt_bitstr(bigint_vector c);

  // i/o functions
  friend istream & operator >> (istream & in, private_key & privkey);
  friend ostream & operator << (ostream & out, const private_key & privkey);

};


class public_key {

private:
  bigint_matrix B;
  int method;
  int print_format;

public:
  // constructor
  public_key() { method=1; print_format=1; }

  // encryption method: 1-message in error vector, 2-message in lattice point
  int get_method(void) { return method; }
  void set_method(int m) { method=m; }

  // print format: 1-plain, 2-matlab
  int get_print_format(void) { return print_format; }
  void set_print_format(int f) { print_format=f; }

  // get the dimention of the basis
  int get_dim(void) { return B.get_no_of_rows(); }

  // encryption functions
  bigint_vector encrypt(bigint_vector v,bigint_vector e);
  bigint_vector encrypt_bitstr(int_vector b);

  // i/o functions
  friend istream & operator >> (istream & in, public_key & pubkey);
  friend ostream & operator << (ostream & out, const public_key & pubkey);
  
  friend class private_key;
};


#endif
