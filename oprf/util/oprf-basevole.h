#ifndef OPRF_BASE_VOLE_H__
#define OPRF_BASE_VOLE_H__

#include "emp-tool/emp-tool.h"
#include "oprf/util/oprf-cope.h"

template <typename IO> class OprfBaseVole {
public:
  int party;
  int m;
  IO *io;
  OprfCope<IO> *cope;
  mpz_class Delta;

  // SENDER
  OprfBaseVole(int party, IO *io, mpz_class Delta) {
    this->party = party;
    this->io = io;
    cope = new OprfCope<IO>(party, io, oprf_P_len);
    this->Delta = Delta;
    cope->initialize(Delta);
  }

  // RECEIVER
  OprfBaseVole(int party, IO *io) {
    this->party = party;
    this->io = io;
    cope = new OprfCope<IO>(party, io, oprf_P_len);
    cope->initialize();
  }

  ~OprfBaseVole() { delete cope; }

  // sender
  void triple_gen_send(std::vector<mpz_class> &share, const int size) {
    cope->extend(share, size);
    mpz_class b = cope->extend();
    sender_check(share, b, size);
  }

  // recver
  void triple_gen_recv(std::vector<mpz_class> &share, std::vector<mpz_class> &x, const int size) {
    x.resize(size);
    GMP_PRG_FP prg;
    for (int i = 0; i < size; i++) x[i] = prg.sample();
    cope->extend(share, x, size);
    mpz_class c = prg.sample();
    mpz_class b = cope->extend(c);
    recver_check(share, x, c, b, size);
  }

  // sender check
  void sender_check(std::vector<mpz_class> &share, mpz_class b, const int size) {
    GMP_PRG_FP prg;
    mpz_class chi = prg.sample();
    uint8_t *chi_de = new uint8_t[oprf_P_len / 8];
    hex_decompose(chi, chi_de);
    io->send_data(chi_de, oprf_P_len / 8);
    io->flush();

    // universal polynomial hash
    mpz_class acc_coeff = chi;
    mpz_class acc_pi = b;

    // inner product
    for (int i = 0; i < size; i++) {
      acc_pi = (acc_pi + acc_coeff * share[i]) % gmp_P;
      acc_coeff = (acc_coeff * chi) % gmp_P;
    }

    io->recv_data(chi_de, oprf_P_len / 8);
    mpz_class xz0 = hex_compose(chi_de);
    io->recv_data(chi_de, oprf_P_len / 8);
    mpz_class xz1 = hex_compose(chi_de);

    xz1 = (xz1 * Delta) % gmp_P;
    acc_pi = (acc_pi + xz1) % gmp_P;
    if (acc_pi != xz0) {
      std::cout << "base VOLE check fails" << std::endl;
      abort();      
    }

    delete[] chi_de;
  }

  // receiver check
  void recver_check(std::vector<mpz_class> &share, std::vector<mpz_class> &x, mpz_class c, mpz_class b, const int size) {
    uint8_t *chi_de = new uint8_t[oprf_P_len / 8];
    io->recv_data(chi_de, oprf_P_len / 8);
    mpz_class chi = hex_compose(chi_de);

    // uni poly hash
    mpz_class acc_coeff = chi;
    mpz_class acc_pi0 = b;
    mpz_class acc_pi1 = c;

    // inner product
    for (int i = 0; i < size; i++) {
      acc_pi0 = (acc_pi0 + acc_coeff * share[i]) % gmp_P;
      acc_pi1 = (acc_pi1 + acc_coeff * x[i]) % gmp_P;
      acc_coeff = (acc_coeff * chi) % gmp_P;
    }

    hex_decompose(acc_pi0, chi_de);
    io->send_data(chi_de, oprf_P_len / 8);
    hex_decompose(acc_pi1, chi_de);
    io->send_data(chi_de, oprf_P_len / 8);
    io->flush();
    delete[] chi_de;
  }

};

#endif
