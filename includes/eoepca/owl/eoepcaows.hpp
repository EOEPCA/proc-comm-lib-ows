#ifndef EOEPCAOWS_EOEPCAOWS_HPP
#define EOEPCAOWS_EOEPCAOWS_HPP

#include "eoepca/IMod.hpp"
#include "eoepca/owl/owsparameter.hpp"

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
        return;
      }

      getParserName = (void (*)(char*, int))dlsym(handle, "getParserName");
      if (!getParserName) {
        setValid(false);
        return;
      }

      parseFromFile = (void (*)(const char*, EOEPCA::OWS::OWSParameter*))dlsym(
          handle, "parseFromFile");
      if (!parseFromFile) {
        setValid(false);
        return;
      }

      parseFromMemory =
          (void (*)(const char*, int, EOEPCA::OWS::OWSParameter*))dlsym(
              handle, "parseFromMemory");
      if (!parseFromMemory) {
        setValid(false);
        return;
      }

      releaseParameter = (void (*)(EOEPCA::OWS::OWSParameter*))dlsym(
          handle, "releaseParameter");
      if (!releaseParameter) {
        setValid(false);
        return;
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
  void (*parseFromFile)(const char*, EOEPCA::OWS::OWSParameter*);
  void (*parseFromMemory)(const char*, int, EOEPCA::OWS::OWSParameter*);
  void (*releaseParameter)(EOEPCA::OWS::OWSParameter*);
};

}  // namespace EOEPCA

#endif  // EOEPCAOWS_EOEPCAOWS_HPP
