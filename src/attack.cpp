#include <fstream>
#include <cstdlib>

#include "Key.h"
#include "Invert.cpp"

private_key privkey;
public_key pubkey;
public_key modkey;

#ifndef dimension
#define dimension 51
#endif

int sigma = 3;
int_vector msg(dimension,dimension,FIXED); 
bigint_vector cipher(dimension,dimension,FIXED);

int_vector generate_msg(){
  int_vector b(dimension,dimension,FIXED);
  int range = 128;
  for (int i=0; i<dimension; i++){
    b[i] = (rand()%(2*range))-range;
  }
  ofstream fout("data/msg");
  fout << b; // write generated message to file
  fout.close();
  return b;
}

void attack(){
  bigfloat_matrix modkey_inv;
  bigfloat_matrix modkey_inv_int;
  bigfloat_vector cipher_f(dimension,dimension,FIXED);
  for (int i=0; i<dimension; i++)  cipher_f[i] = cipher[i];

  for (int i = 0; i < dimension; ++i)
  {
    cipher[i]+=sigma;
    cipher[i]%=2*sigma;
  }

  bigint det,detmod;
  pubkey.get_determinant(det);
  modkey.B = pubkey.B;
  modkey.apply_mod(2*sigma);
  modkey.get_determinant(detmod);

  cout << detmod << " " << detmod%(2*sigma) << endl;
  cout << det << " " << det%(2*sigma) << endl;

  invert_bigint_matrix(modkey.B,modkey_inv);
  bigfloat_vector m2sigma(dimension,dimension,FIXED);
  m2sigma = modkey_inv*cipher_f;
  for (int i=0; i<dimension; i++)
    m2sigma[i] = (m2sigma[i]>0) ? floor(0.5+m2sigma[i]) : ceil(-0.5+m2sigma[i]);

  for (int i = 0; i < dimension; ++i)
  {
    cout << m2sigma[i] << endl;
  }
  
}

main() {
  // cin >> dimension; cout << dimension << " ";
  msg = generate_msg();

  privkey.generate(dimension);         // generate private key

  ofstream fout("data/priv_key");
  fout << privkey;               // write private key to file
  fout.close();

  privkey.make_public(pubkey);   // initialize public key

  ofstream fin("data/pub_basis");
  fin << pubkey; // read public key
  fin.close();

  pubkey.set_method(0);
  cipher = pubkey.encrypt_bitstr(msg);   // encrypt message

  ofstream fout1("data/cipher");
  fout1 << cipher; // write generated message to file
  fout1.close();


  int_vector d(dimension,dimension,FIXED);
  privkey.set_method(pubkey.get_method());
  d = privkey.decrypt_bitstr(cipher);  // decrypt message

  if (!msg.equal(d))                // compare with the original
    cerr << "Ooops!" << endl;

  attack();
}
