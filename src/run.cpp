

#include <eoepca/owl/eoepcaows.hpp>
#include <eoepca/owl/owsparameter.hpp>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <optional>
#include <string>

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

  std::unique_ptr<EOEPCA::OWS::OWSEntry,
                  std::function<void(EOEPCA::OWS::OWSEntry*)>>
      theParams(lib->parseFromFile(argv[1]), lib->releaseParameter);

  if (theParams) {
    std::cout << "********************************\n";
    std::cout << theParams->getPackageIdentifier() << "\n";

    for (auto& off : theParams->getOfferings()) {
      for (auto& y : off->getContents()) {
        std::cout << "\t" << y.code << " " << y.href << "\n";
      }

      for (auto& proc : off->getProcessDescription()) {
        std::cout << proc->getIdentifier() << "\n";
        std::cout << proc->getTitle() << "\n";
        std::cout << proc->getAbstract() << "\n";

        std::cout << "theParams SIZE INPUT: " << proc->getInputs().size()
                  << "\n";
        std::cout << "theParams SIZE OUTPUT: " << proc->getOutputs().size();
      }
    }
  }

  return 0;
}
