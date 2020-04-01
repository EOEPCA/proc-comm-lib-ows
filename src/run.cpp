


#include <eoepca/owl/eoepcaows.hpp>

#include <eoepca/owl/owsparameter.hpp>

#include <iostream>
#include <memory>

int main(int argc, const char** argv) {
  if (argc == 1) {
    std::cerr << "arg1: ows file path";
    return 1;
  }

  auto lib = std::make_unique<EOEPCA::EOEPCAows>("./libeoepcaows.so");
  if (!lib->IsValid()) {
    lib = std::make_unique<EOEPCA::EOEPCAows>("./libeoepcaows.dylib");
  }

  std::cout << "LIB version: " << lib->version() << "\n";
  int maxLen = 1024;
  auto theName = std::make_unique<char[]>(maxLen);
  lib->getParserName(theName.get(), maxLen);

  std::cout << "LIB name: " << theName.get() << "\n";

  std::cout << "Run: \n";

  EOEPCA::OWS::OWSParameter* p{nullptr};

  lib->parseFromFile(argv[1],p);

  delete p;


  return 0;
}
