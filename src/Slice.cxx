#include "Slice.hpp"
#include "Util.hpp"
#include <cc4x.hpp>

namespace Slice{

  sliceDim getDimensions( std::vector<int64_t> lens
                        , std::vector<int64_t> pp){
    if ( pp.size() != lens.size() ) {
      LOG() << pp.size() << " " << lens.size() << std::endl;
      THROW("Invalid input!");
    }
    auto order(lens.size());
    std::vector< std::vector<int64_t> > srcBegin(1, std::vector<int64_t>(order));
    std::vector< std::vector<int64_t> > srcEnd(1, std::vector<int64_t>(order));

    for (size_t i(0); i < order; i++){
      // edge gets slices....
      if (pp[i] > 0 && pp[i] < lens[i]) {
        auto cpyBegin(srcBegin);
        auto cpyEnd(srcEnd);
        // lower part
        for (auto &e: srcBegin) e[i] = 0;
        for (auto &e: srcEnd) e[i] = pp[i];
        // upper part
        for (auto &e: cpyBegin) e[i] = pp[i];
        for (auto &e: cpyEnd) e[i] = lens[i];
        // fuse the two vectors
        srcBegin.insert(srcBegin.end(), cpyBegin.begin(), cpyBegin.end());
        srcEnd.insert(srcEnd.end(), cpyEnd.begin(), cpyEnd.end());
      }
      else {
        for (auto &e: srcBegin) e[i] = 0;
        for (auto &e: srcEnd) e[i] = lens[i];
      }
    }
    // destination always begins with 0, right
    std::vector< std::vector<int64_t> >
      dstBegin(srcBegin.size(), std::vector<int64_t>(order));
    std::vector< std::vector<int64_t> > dstEnd(srcEnd);
    for (size_t s(0); s < dstEnd.size(); s++)
    for (size_t o(0); o < order; o++)
      dstEnd[s][o] = srcEnd[s][o] - srcBegin[s][o];

    if (cc4x::verbose) {
      LOG() << "slicing into " << srcBegin.size() << " tensors" << std::endl;
      for (size_t i(0); i < srcBegin.size(); i++){
      for (size_t o(0); o < order; o++)
        LOG() << "[" << srcBegin[i][o] << " - " << srcEnd[i][o] << "]";
      LOG() << std::endl;
      }
      LOG() << "=====\n";
      for (size_t i(0); i < dstBegin.size(); i++){
      for (size_t o(0); o < order; o++)
        LOG() << "[" << dstBegin[i][o] << " - " << dstEnd[i][o] << "]";
      LOG()  << std::endl;
      }
    }
    return {srcBegin, srcEnd, dstBegin, dstEnd};
  }


  void run(input const& in, output& out){
    // sanity checks
    if (in.I == NULL || in.I == (tensor<Complex>*)0xfafa) {
      THROW("Input of Slice not valid");
    }
    auto lens(in.I->lens);
    auto order(lens.size());
    auto pp(in.partitionPoint);
    auto slices = getDimensions(lens, pp);

    if (slices.dstBegin.size() == 1) { THROW("Nothing to slice. Wrong input!"); }
    if (slices.dstBegin.size() > 4) {
      THROW("We cannot slice into more that 4 objects");
    }


    auto dummyA = new tensor<Complex>(
      order, slices.dstEnd[0], cc4x::kmesh->getNZC(order), cc4x::world, "Slice1"
    );
    dummyA->slice(slices.dstBegin[0], slices.dstEnd[0], 0.0, *in.I, slices.srcBegin[0], slices.srcEnd[0], 1.0);
    *out.A = dummyA;

    auto dummyB = new tensor<Complex>(
      order, slices.dstEnd[1], cc4x::kmesh->getNZC(order), cc4x::world, "Slice2"
    );
    dummyB->slice(slices.dstBegin[1], slices.dstEnd[1], 0.0, *in.I, slices.srcBegin[1], slices.srcEnd[1], 1.0);
    *out.B = dummyB;

    if (slices.dstBegin.size() == 2) return;
    auto dummyC = new tensor<Complex>(
      order, slices.dstEnd[2], cc4x::kmesh->getNZC(order), cc4x::world, "Slice3"
    );
    dummyC->slice(slices.dstBegin[2], slices.dstEnd[2], 0.0, *in.I, slices.srcBegin[2], slices.srcEnd[2], 1.0);
    *out.C = dummyC;

    auto dummyD = new tensor<Complex>(
      order, slices.dstEnd[3], cc4x::kmesh->getNZC(order), cc4x::world, "Slice4"
    );
    dummyD->slice(slices.dstBegin[3], slices.dstEnd[3], 0.0, *in.I, slices.srcBegin[3], slices.srcEnd[3], 1.0);
    *out.D = dummyD;

    return;
  }

}
