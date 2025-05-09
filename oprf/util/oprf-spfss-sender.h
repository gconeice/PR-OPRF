#ifndef OPRF_SPFSS_SENDER_FP_H__
#define OPRF_SPFSS_SENDER_FP_H__
#include "emp-zk/emp-vole/utility.h"
#include "oprf/util/gmp-fp.h"
#include "oprf/util/gmp-prg-fp.h"
#include <emp-ot/emp-ot.h>
#include <emp-tool/emp-tool.h>
#include <iostream>
#include <vector>

using namespace emp;

template <typename IO> class OprfSpfssSenderFp {
public:
  block seed;
  block *ggm_tree, *m;
  mpz_class delta;
  mpz_class secret_sum;  
  mpz_class *last_layer;
  // std::vector<mpz_class> last_layer;
  IO *io;
  int depth;
  int leave_n;
  PRG prg;

  OprfSpfssSenderFp(IO *io, int depth_in) {
    initialization(io, depth_in);
    prg.random_block(&seed, 1);
  }

  void initialization(IO *io, int depth_in) {
    this->io = io;
    this->depth = depth_in;
    this->leave_n = 1 << (this->depth - 1);
    m = new block[(depth - 1) * 2];
  }

  ~OprfSpfssSenderFp() { delete[] m; }

  // send the nodes by oblivious transfer
  void compute(__uint128_t *ggm_tree_mem, mpz_class *last, const mpz_class &secret,
               const mpz_class &gamma) {
    this->delta = secret;
    last_layer = last;
    ggm_tree_gen(m, m + depth - 1, ggm_tree_mem, gamma);
  }

  // send the nodes by oblivious transfer
  template <typename OT> void send(OT *ot, IO *io2, int s) {
    ot->send(m, &m[depth - 1], depth - 1, io2, s);
    std::vector<uint8_t> vvv(48);
    hex_decompose(secret_sum, &vvv[0]);
    io2->send_data(&vvv[0], 48);
    io2->flush();
  }

  // generate GGM tree from the top
  void ggm_tree_gen(block *ot_msg_0, block *ot_msg_1, __uint128_t *ggm_tree_mem,
                    const mpz_class &gamma) {
    this->ggm_tree = (block *)ggm_tree_mem;
    TwoKeyPRP *prp = new TwoKeyPRP(zero_block, makeBlock(0, 1));
    prp->node_expand_1to2(ggm_tree, seed);
    ot_msg_0[0] = ggm_tree[0];
    ot_msg_1[0] = ggm_tree[1];
    for (int h = 1; h < depth - 1; ++h) {
      ot_msg_0[h] = ot_msg_1[h] = zero_block;
      int sz = 1 << h;
      for (int i = sz - 2; i >= 0; i -= 2) {
        prp->node_expand_2to4(&ggm_tree[i * 2], &ggm_tree[i]);
        ot_msg_0[h] = ot_msg_0[h] ^ ggm_tree[i * 2];
        ot_msg_0[h] = ot_msg_0[h] ^ ggm_tree[i * 2 + 2];
        ot_msg_1[h] = ot_msg_1[h] ^ ggm_tree[i * 2 + 1];
        ot_msg_1[h] = ot_msg_1[h] ^ ggm_tree[i * 2 + 3];
      }
    }
    delete prp;
    secret_sum = 0;
    // last_layer.resize(leave_n);
    for (int i = 0; i < leave_n; ++i) {
      GMP_PRG_FP layer_prg(&ggm_tree[i]);
      last_layer[i] = layer_prg.sample();
      secret_sum += last_layer[i];
    }
    secret_sum %= gmp_P;
    secret_sum = gmp_P - secret_sum;
    secret_sum = (secret_sum + gamma) % gmp_P;
  }


  // consistency check: Protocol PI_spsVOLE
  void consistency_check_msg_gen(mpz_class &V, block seed) {
    GMP_PRG_FP chalprg(&seed);
    mpz_class chal = chalprg.sample();
    V = 0;
    mpz_class coeff = chal;
    for (int i = 0; i < leave_n; i++) {
      V = (V + coeff * last_layer[i]) % gmp_P;
      coeff = (coeff * chal) % gmp_P;
    }
  }

  // correctness debug
  void correctness_check(IO *io2) {
    std::vector<uint8_t> vvv(48);
    io2->recv_data(&vvv[0], 48);
    mpz_class beta = hex_compose(&vvv[0]);
    int choice_pos;
    io2->recv_data(&choice_pos, sizeof(int));

    for (int i = 0; i < leave_n; i++) {
      io2->recv_data(&vvv[0], 48);
      mpz_class v = hex_compose(&vvv[0]);
      if (i == choice_pos) {
        mpz_class w = (delta * beta + last_layer[i]) % gmp_P;
        if (w != v) {
          cout << "error in the hot position!" << endl;
          cout << delta << ' ' << beta << ' ' << last_layer[i] << ' ' << v << endl;
          abort();
        }
      } else {
        if (v != last_layer[i]) {
          cout << "error in the non-hot possion!" << ' ' << i << endl;
          abort();
        }
      }
    }

    cout << "correctness test pass" << endl;
  }
};

#endif
