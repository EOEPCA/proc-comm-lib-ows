
#ifndef EOEPCAOWS_OWSPARAMETER_HPP
#define EOEPCAOWS_OWSPARAMETER_HPP

#include <iostream>
#include <list>
#include <memory>
#include <sstream>
#include <string>

namespace EOEPCA::OWS {

class Descriptor {
 protected:
  std::string identifier{""};
  std::string title{""};
  std::string abstract{""};
  std::string version{""};

 public:
  Descriptor() = default;
  Descriptor(const Descriptor &other)
      : identifier(other.identifier),
        title(other.title),
        version(other.version),
        abstract(other.abstract) {}

  Descriptor(Descriptor &&other)
      : identifier(std::move(other.identifier)),
        title(std::move(other.title)),
        abstract(std::move(other.abstract)),
        version(std::move(other.version)) {}

  Descriptor &operator=(const Descriptor &other) {
    if (this == &other) return *this;

    identifier = other.identifier;
    title = other.title;
    abstract = other.abstract;
    version = other.version;

    return *this;
  }

  Descriptor &operator=(Descriptor &&other) noexcept(false) {
    if (this == &other) return *this;

    identifier = std::move(other.identifier);
    title = std::move(other.title);
    abstract = std::move(other.abstract);
    version = std::move(other.version);

    return *this;
  }

  virtual ~Descriptor() = default;

 public:
  void setVersion(std::string pVersion) {
    Descriptor::version = std::move(pVersion);
  }

  const std::string &getIdentifier() const { return identifier; }
  void setIdentifier(std::string pIdentifier) {
    Descriptor::identifier = std::move(pIdentifier);
  }
  const std::string &getTitle() const { return title; }
  void setTitle(const std::string &title) { Descriptor::title = title; }
  const std::string &getAbstract() const { return abstract; }
  void setAbstract(const std::string &abstract) {
    Descriptor::abstract = abstract;
  }
};

class Occurs {
 protected:
  int minOccurs{1};
  int maxOccurs{0};

 public:
  Occurs() = default;

  Occurs(const Occurs &) = default;
  Occurs(Occurs &&) = default;

  Occurs &operator=(const Occurs &other) {
    if (this == &other) return *this;
    minOccurs = other.minOccurs;
    maxOccurs = other.maxOccurs;
    return *this;
  }

  Occurs &operator=(Occurs &&other) noexcept(false) {
    if (this == &other) return *this;
    minOccurs = other.minOccurs;
    maxOccurs = other.maxOccurs;
    return *this;
  }

  virtual ~Occurs() = default;

 public:
  int getMinOccurs() const { return minOccurs; }
  int getMaxOccurs() const { return maxOccurs; }

  void setMinOccurs(std::string pMinOccurs) {
    if (pMinOccurs.empty()) {
      minOccurs = 0;
    } else {
      std::istringstream iss(pMinOccurs);
      iss >> minOccurs;
      if (iss.fail()) {
        // TODO: raise the exception
      }
    }
  }

  void setMaxOccurs(std::string pMaxOccurs) {
    if (pMaxOccurs.empty()) {
      maxOccurs = 0;
    } else {
      std::istringstream iss(pMaxOccurs);
      iss >> maxOccurs;
      if (iss.fail()) {
        // TODO: raise the exception
      }
    }
  }

  void setMinOccurs(int minOccurs) { Occurs::minOccurs = minOccurs; }
  void setMaxOccurs(int maxOccurs) { Occurs::maxOccurs = maxOccurs; }
};

class Param : public Descriptor, public Occurs {
 public:
  Param() : Descriptor(), Occurs(){};
  Param(const Param &other) noexcept(false)
      : Descriptor(other), Occurs(other) {}
  Param(Param &&other) noexcept(false)
      : Descriptor(std::move(other)), Occurs(std::move(other)) {}

  Param &operator=(const Param &other) {
    if (this == &other) return *this;

    Descriptor::operator=(other);
    Occurs::operator=(other);

    return *this;
  }

  Param &operator=(Param &&other) noexcept(false) {
    if (this == &other) return *this;

    Descriptor::operator=(std::move(other));
    Occurs::operator=(std::move(other));

    return *this;
  }

  ~Param() override = default;

  virtual std::string getType() = 0;
};

class LiteralData final : public Param {
 private:
 public:
  std::string getType() override { return std::string("LiteralData"); }
};

class BoundingBoxData final : public Param {
 private:
 public:
  std::string getType() override { return std::string("BoundingBoxData"); }
};

class ComplexData final : public Param {
 private:
 public:
  std::string getType() override { return std::string("ComplexData"); }
};

struct Content {
  Content(std::string pCode, std::string pHref)
      : code(std::move(pCode)), href(std::move(pHref)) {}

  std::string href{""};
  std::string code{""};
  std::string tag{""};
};

class OWSParameter final : public Descriptor {
  std::string packageIdentifier{""};

  std::list<std::unique_ptr<Param>> inputs;
  std::list<std::unique_ptr<Param>> outputs;

  std::list<Content> contents{};

 public:
  OWSParameter() = default;
  OWSParameter(const OWSParameter &) = delete;
  OWSParameter(OWSParameter &&) = delete;

  ~OWSParameter() override = default;

 public:
  void setPackageIdentifier(std::string pPackageIdentifier) {
    OWSParameter::packageIdentifier = std::move(pPackageIdentifier);
  }

  const std::string &getPackageIdentifier() const { return packageIdentifier; }

  const std::list<Content> &getContents() const { return contents; }

  void addInput(Param *param) {
    if (param) {
      inputs.emplace_back(param);
    }
  }

  void addContent(std::string code, std::string href) {
    if (!code.empty()) {
      contents.emplace_back(code, href);
    }
  }
};

class Ows {};

}  // namespace EOEPCA::OWS
#endif  // EOEPCAOWS_OWSPARAMETER_HPP
