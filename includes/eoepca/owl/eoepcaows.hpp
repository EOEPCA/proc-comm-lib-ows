#ifndef EOEPCAOWS_EOEPCAOWS_HPP
#define EOEPCAOWS_EOEPCAOWS_HPP

#include "eoepca/IMod.hpp"

namespace EOEPCA {
class EOEPCAows : protected mods::IModInterface {
 public:
  EOEPCAows() = delete;
  explicit EOEPCAows(const std::string& path)
      : mods::IModInterface(path),
        version(nullptr),
        getParserName(nullptr),
        parseFromFile(nullptr),
        parseFromMemory(nullptr) {
    if (isValid()) {
      version = (long (*)(void))dlsym(handle, "version");
      if (!version) {
        setValid(false);
      }

      getParserName = (void (*)(char*, int))dlsym(handle, "getParserName");
      if (!getParserName) {
        setValid(false);
      }

      parseFromFile = (void (*)(const char*))dlsym(handle, "parseFromFile");
      if (!parseFromFile) {
        setValid(false);
      }

      parseFromMemory =
          (void (*)(const char*, int))dlsym(handle, "parseFromMemory");
      if (!parseFromMemory) {
        setValid(false);
      }
    }
  }

 public:
  void ResetError() { this->resetError(); }

  bool IsValid() { return this->isValid(); }
  std::string GetLastError() {
    return std::move(std::string(this->getLastError()));
  };

  // interface
 public:
  long (*version)(void);
  void (*getParserName)(char*, int);
  void (*parseFromFile)(const char*);
  void (*parseFromMemory)(const char*, int);
};

}  // namespace EOEPCA

#endif  // EOEPCAOWS_EOEPCAOWS_HPP
