#include "Util.hpp"

#ifndef CCSD_DEFINED
#define CCSD_DEFINED


namespace Ccsd {
  struct input {
    tensor<Complex> *Vhhhh;
    tensor<Complex> *Vhhhp;
    tensor<Complex> *Vhhph;
    tensor<Complex> *Vhhpp;
    tensor<Complex> *Vhphp;
    tensor<Complex> *Vhppp;
    tensor<Complex> *Vphhh;
    tensor<Complex> *Vphhp;
    tensor<Complex> *Vphph;
    tensor<Complex> *Vphpp;
    tensor<Complex> *Vpphh;
    tensor<Complex> *Vpphp;
    tensor<Complex> *Vppph;
    tensor<Complex> *Vpppp;
    tensor<Complex> *epsi;
    tensor<Complex> *epsa;
  };
  struct output {
  };
  void run(input const& in, output &out);

}


#endif
