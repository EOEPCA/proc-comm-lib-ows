#ifndef EOEPCAOWS_EOEPCAOWS_HPP
#define EOEPCAOWS_EOEPCAOWS_HPP

#include "eoepca/IMod.hpp"

namespace EOEPCA {
class EOEPCAows : protected mods::IModInterface {
 public:
  EOEPCAows() = delete;
  explicit EOEPCAows(const std::string& path)
      : mods::IModInterface(path), version(nullptr), getParserName(nullptr) {
    if (isValid()) {
      version = (long (*)(void))dlsym(handle, "version");
      if (!version) {
        setValid(false);
      }

      getParserName = (void (*)(char*, int))dlsym(handle, "getParserName");
      if (!version) {
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
};

}  // namespace EOEPCA

#endif  // EOEPCAOWS_EOEPCAOWS_HPP
