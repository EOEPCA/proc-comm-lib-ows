
#include <cstring>
#include <eoepca/owl/owsparameter.hpp>
#include <fstream>
#include <memory>
#include <sstream>
#include <iostream>

#include "includes/parser.hpp"

extern "C" long version() { return 1; }

extern "C" void getParserName(char* buffer, int maxSize ) {
  auto parser = std::make_unique<EOEPCA::Parser>();

  int val = parser->getName().size();
  int len = (maxSize < val ? val : maxSize - 1);

  std::memset(buffer, '\0', maxSize);
  std::strncpy(buffer, parser->getName().c_str(), len);
}


extern "C" EOEPCA::OWS::OWSParameter*  parseFromFile(const char* fileName) {

  auto parser = std::make_unique<EOEPCA::Parser>();

  std::ifstream infile(fileName);
  if (infile.good()) {
    std::stringstream sBuffer;
    sBuffer << infile.rdbuf();

    parser->parseXml(sBuffer.str().c_str(), sBuffer.str().size());

  } else {
    std::string err("failed to open ");
    err.append(fileName);
    throw std::runtime_error(err);
  }


  auto y=new  EOEPCA::OWS::OWSParameter();
  y->setTitle("parseFromMemory");

  return y;

}

extern "C" EOEPCA::OWS::OWSParameter* parseFromMemory(const char* xmlBuffer, int size) {
  auto parser = std::make_unique<EOEPCA::Parser>();
  parser->parseXml(xmlBuffer, size);

  auto y=new  EOEPCA::OWS::OWSParameter();
  y->setTitle("parseFromMemory");

  return y;
}



extern "C" void releaseParameter(EOEPCA::OWS::OWSParameter* parameter) {
  delete parameter;
}
