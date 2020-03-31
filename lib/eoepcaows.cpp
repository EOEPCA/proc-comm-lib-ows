#include "includes/parser.hpp"

#include <cstring>
#include <memory>

extern "C" long version() { return 1; }

extern "C" void  getParserName(char* buffer,int maxSize){
  auto parser=std::make_unique<EOEPCA::Parser>();

  int val=parser->getName().size();
  int len=(maxSize<val?val:maxSize-1);

  std::memset(buffer,'\0',maxSize);
  std::strncpy(buffer,parser->getName().c_str(),len);
}
