

#include <eoepca/owl/eoepcaows.hpp>
#include <eoepca/owl/owsparameter.hpp>
#include <functional>
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

  std::unique_ptr<EOEPCA::OWS::OWSParameter,
                  std::function<void(EOEPCA::OWS::OWSParameter*)> >
      theParams(lib->parseFromFile(argv[1]), lib->releaseParameter);

  if (theParams) {
    std::cout << "********************************\n";
    std::cout << theParams->getPackageIdentifier() << "\n";

    std::cout << theParams->getIdentifier() << "\n";
    std::cout << theParams->getTitle() << "\n";
    std::cout << theParams->getAbstract() << "\n";


    for (auto& y : theParams->getContents()) {
      std::cout << "\t" << y.code << " " << y.href << "\n";
    }
  }

  return 0;
}
