#include "Key.h"
#include <fstream>
#include <cstdlib>

private_key privkey;
public_key pubkey;

main() {
  int dimension = 100;
  privkey.generate(dimension);         // generate private key

  ofstream fout("key.100");
  fout << privkey;               // write private key to file
  fout.close();

  privkey.make_public(pubkey);   // initialize public key

  int_vector b(dimension,dimension,FIXED);   // int_vector defined in Key.h
  for (int i=0; i<dimension; i++)
    b[i] = rand()&1;             // a random message

  bigint_vector c(dimension,dimension,FIXED); // also defined in Key.h
  pubkey.set_method(1);
  c = pubkey.encrypt_bitstr(b);   // encrypt message

  int_vector d(dimension,dimension,FIXED);
  privkey.set_method(pubkey.get_method());
  d = privkey.decrypt_bitstr(c);  // decrypt message

  if (!b.equal(d))                // compare with the original
    cerr << "Ooops!" << endl;
}
