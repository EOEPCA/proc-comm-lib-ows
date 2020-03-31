

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include <eoepca/owl/eoepcaows.hpp>
#include <iostream>
#include <memory>

int main(int argc, const char** argv) {
  auto lib = std::make_unique<EOEPCA::EOEPCAows>("./libeoepcaows.so");
  if (!lib->IsValid()) {
    lib = std::make_unique<EOEPCA::EOEPCAows>("./libeoepcaows.dylib");
  }

  std::cout << "LIB version: " << lib->version() << "\n";
  int maxLen = 1024;
  auto theName = std::make_unique<char[]>(maxLen);
  lib->getParserName(theName.get(), maxLen);

  std::cout << "LIB name: " << theName << "\n";

  return 0;
}
