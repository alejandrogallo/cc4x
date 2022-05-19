#include "Read.hpp"
#include <cc4x.hpp>

namespace Read{

  yamlData readYaml(YAML::Node yamlFile){
    yamlData y;
     
    if (yamlFile["elements"]){
      auto elements = yamlFile["elements"];
      if (elements["type"]) {
        y.fileType = elements["type"].as<std::string>();
      }
    } 
    if (yamlFile["dimensions"]) {
      auto dimensions = yamlFile["dimensions"];
      y.order = dimensions.size();
      for (auto d: dimensions)
        if (d["length"]) y.lens.push_back( d["length"].as<int>() );
    }
    if (yamlFile["scalarType"])
      y.scalarType = yamlFile["scalarType"].as<std::string>();
    if (yamlFile["metaData"]){
      auto meta = yamlFile["metaData"];
      if (meta["No"]) y.No = meta["No"].as<int>(); 
      if (meta["Nv"]) y.Nv = meta["Nv"].as<int>();
      if (meta["kMesh"]) y.kMesh = meta["kMesh"].as<ivec>();
      if (meta["halfGrid"]) y.halfGrid = meta["halfGrid"].as<int>();
    }
    return y;
  }


  void run(input const& in, output& out){
    YAML::Node yamlFile;
    try { yamlFile = YAML::LoadFile(in.fileName); }
    catch ( const YAML::Exception &cause){
      THROW("file not present or badly formatted yaml file given");
    }
    yamlData y = readYaml(yamlFile);
    if (y.No) cc4x::No = y.No;
    if (y.Nv) cc4x::Nv = y.Nv;
    // If kMesh is already given for this calculation; make sure that meshes agree
    if (!cc4x::kmesh) cc4x::kmesh = new kMesh(y.kMesh);
    else
      if (y.kMesh != cc4x::kmesh->mesh) {
        THROW("inconsistent meshes in input-yaml");
      }
    auto dataFile 
      = in.fileName.substr(0, in.fileName.find_last_of(".")) + ".elements";
    std::ifstream file;
    file.open(dataFile, std::ifstream::in);
    if (!file.is_open()) { THROW("element file not present"); }

    //TODO: maybe not the most professional way...
    //if (file.is_open()) std::cout << "juhuu" << std::endl;
    //else std::cout << "nee" << std::endl;


    auto d = new CTF::bsTensor<Complex>(
      y.order, y.lens, cc4x::kmesh->getNZC(y.order), cc4x::dw
    );

    //TODO: no data in our tensors, yet
    //TODO: the following is pretty crazy hardcoding:
    //      it assumes real data into complex tensor
    if (y.fileType == "TextFile"){
      std::vector<Complex> data;
      std::string line;
      size_t count(0);
      while (std::getline(file, line)){
        double val(std::stod(line));
        data.push_back({val,0.0});
        count++;
      }
      file.close();
      auto Np(cc4x::No + cc4x::Nv);
      std::vector<int64_t> idx(Np);
      std::iota(idx.begin(), idx.end(), 0);
      d->write(Np, idx.data(), data.data());
    } else {
      MPI_File file;
      MPI_File_open(
        cc4x::dw->comm, dataFile.c_str(), MPI_MODE_RDONLY, MPI_INFO_NULL, &file
      );
      d->read_dense_from_file(file);
      MPI_File_close(&file);
    }
    *out.T = d;
  }

}

