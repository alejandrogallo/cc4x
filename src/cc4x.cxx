#include <string>
#include <iostream>

#include <ctf.hpp>
#include <CLI11.hpp>

#include <cc4x.hpp>
#include <Read.hpp>
#include <Kmesh.hpp>
#include <Slice.hpp>
#include <Integrals.hpp>
#include <Drccd.hpp>
#include <Ccsd.hpp>
#include <Ueg.hpp>

// this is a insane hack.once in the lifetime of the universe, we will fail
// ever tried. ever failed. no matter. try again. fail again. fail better.
#define NULL_TENSOR ((tensor<Complex>*)0xfafa)

bool cc4x::verbose = 0;
bool cc4x::complexT;
int cc4x::No;
int cc4x::Nv;
int cc4x::iterations;
CTF::World * cc4x::dw = NULL;
kMesh * cc4x::kmesh = NULL;

void printSystem(){
  if (cc4x::complexT) { LOG() << "Working with complex Integrals\n"; }
  else { LOG() << "Working with real Integrals\n"; }
  LOG() << "No: " << cc4x::No << " , Nv: " << cc4x::Nv << "\n";
  LOG() << "kMesh: " << cc4x::kmesh->mesh[0] << " " << cc4x::kmesh->mesh[1] << " " << cc4x::kmesh->mesh[2] << "\n";
}


int main(int argc, char **argv){
  MPI_Init(&argc, &argv);
  std::string usage("cc4x: Either provide rs && No && Nv\n");
  usage += "      or provide CoulombVertex.yaml\n";
  usage += "      and EigenEnergies.yaml in the current directory\n";
  CLI::App app{usage};
  double rs(-1.0);
  bool drccd(false);
  bool ccsd(true);
  app.add_option("-o, --occupied", cc4x::No
                , "Number of occupied orbitals\n")->default_val(0);
  app.add_option("-v, --virtuals"
                , cc4x::Nv, "Number of virtual  orbitals\n")->default_val(0);
  app.add_option("-r, --wignerSeitz"
                , rs, "Wigner-Seitz radius\n")->default_val(rs);
  app.add_flag("-d, --drccd", drccd, "Calculate drccd amplitude equations\n");
  app.add_flag("-c, --ccsd", ccsd, "Calculate ccsd amplitude equations\n");
  app.add_option("-i, --iterations", cc4x::iterations
                , "Number of SCF iterations\n")->default_val(10);
  try {
    CLI11_PARSE(app, argc, argv);
  } catch(const CLI::ParseError &e) {
    int retval = app.exit(e);
    MPI_Finalize();
    return retval;
  }


  cc4x::dw = new CTF::World();

  tensor<Complex> *eps = NULL_TENSOR;
  tensor<Complex> *coulombVertex = NULL_TENSOR;

  tensor<Complex> *epsi = NULL_TENSOR;
  tensor<Complex> *epsa = NULL_TENSOR;

  tensor<Complex> *hhVertex = NULL_TENSOR;
  tensor<Complex> *phVertex = NULL_TENSOR;
  tensor<Complex> *hpVertex = NULL_TENSOR;
  tensor<Complex> *ppVertex = NULL_TENSOR;

  tensor<Complex> *Vhhhh = NULL_TENSOR;
  tensor<Complex> *Vhhhp = NULL_TENSOR;
  tensor<Complex> *Vhhph = NULL_TENSOR;
  tensor<Complex> *Vhhpp = NULL_TENSOR;
  tensor<Complex> *Vhphp = NULL_TENSOR;
  tensor<Complex> *Vhppp = NULL_TENSOR;
  tensor<Complex> *Vphhh = NULL_TENSOR;
  tensor<Complex> *Vphhp = NULL_TENSOR;
  tensor<Complex> *Vphph = NULL_TENSOR;
  tensor<Complex> *Vphpp = NULL_TENSOR;
  tensor<Complex> *Vpphh = NULL_TENSOR;
  tensor<Complex> *Vpphp = NULL_TENSOR;
  tensor<Complex> *Vppph = NULL_TENSOR;
  tensor<Complex> *Vpppp = NULL_TENSOR;



  try {
    if (rs < 0){
      Read::getAmplitudesType("CoulombVertex.yaml");
      {
        LOG() << "read eigen" << std::endl;
        Read::input in({"EigenEnergies.yaml"});
        Read::output out({&eps});
        Read::run(in, out);
      }
      {
        LOG() << "read coulomb" << std::endl;
        Read::input in({"CoulombVertex.yaml"});
        Read::output out({&coulombVertex});
        Read::run(in, out);
      }
    } else
    {
      if (cc4x::No == 0 || cc4x::Nv == 0) {
        THROW("Setting rs > 0 requires specification of No && Nv");
      }
      Ueg::input in({cc4x::No, cc4x::Nv, rs});
      Ueg::output out({&coulombVertex, &eps});
      Ueg::run(in, out);
    }
    printSystem();
    {
      LOG() << "slice eps" << std::endl;
      Slice::input in({eps, {cc4x::No}});
      Slice::output out({&epsi, &epsa});
      Slice::run(in, out);
    }
    {
      LOG() << "slice ck" << std::endl;
      Slice::input in({coulombVertex, {0, cc4x::No, cc4x::No}});
      Slice::output out({&hhVertex, &phVertex, &hpVertex, &ppVertex});
      Slice::run(in, out);
    }
    {
      LOG()<< "eval Integrals" << std::endl;
      Integrals::input in({hhVertex, phVertex, hpVertex, ppVertex});
      Integrals::output out({&Vhhhh, &Vhhhp, &Vhhph, &Vhhpp, &Vhphp, &Vhppp, &Vphhh, &Vphhp, &Vphph, &Vphpp, &Vpphh, &Vpphp, &Vppph, &Vpppp});
      Integrals::run(in, out);
    }
    if (drccd)
    {
      LOG() << "drccd" << std::endl;
      Drccd::input in({Vpphh, Vphhp, Vhhpp, epsi, epsa});
      Drccd::output out({});
      Drccd::run(in, out);
    }
    if (ccsd)
    {
      LOG() << "ccsd" << std::endl;
      Ccsd::input in({Vhhhh, Vhhhp, Vhhph, Vhhpp, Vhphp, Vhppp, Vphhh, Vphhp, Vphph, Vphpp, Vpphh, Vpphp, Vppph, Vpppp, epsi, epsa});
      Ccsd::output out({});
      Ccsd::run(in, out);
    }

  } catch (...) {
    LOG() << "WHAT THE FUCK" << std::endl;
  }

  MPI_Finalize();
  return 0;
}
