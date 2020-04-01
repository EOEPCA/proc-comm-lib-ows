
#ifndef EOEPCAOWS_OWSPARAMETER_HPP
#define EOEPCAOWS_OWSPARAMETER_HPP

#include <list>
#include <memory>
#include <string>
#include <iostream>

namespace EOEPCA::OWS {

class Descriptor {
 protected:
  std::string identifier{""};
  std::string title{""};
  std::string abstract{""};

 public:
  Descriptor() = default;
  Descriptor(const Descriptor &other)
      : identifier(other.identifier),
        title(other.title),
        abstract(other.abstract) {}

  Descriptor(Descriptor &&other)
      : identifier(std::move(other.identifier)),
        title(std::move(other.title)),
        abstract(std::move(other.abstract)) {}

  Descriptor &operator=(const Descriptor &other) {
    if (this == &other) return *this;

    identifier = other.identifier;
    title = other.title;
    abstract = other.abstract;

    return *this;
  }

  Descriptor &operator=(Descriptor &&other) noexcept(false) {
    if (this == &other) return *this;

    identifier = std::move(other.identifier);
    title = std::move(other.title);
    abstract = std::move(other.abstract);

    return *this;
  }

  virtual ~Descriptor() = default;

 public:
  const std::string &getIdentifier() const { return identifier; }
  void setIdentifier(const std::string &identifier) {
    Descriptor::identifier = identifier;
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
  void setMinOccurs(int minOccurs) { Occurs::minOccurs = minOccurs; }
  int getMaxOccurs() const { return maxOccurs; }
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

class Ows {};

class OWSParameter final : public Descriptor {
  std::list<std::unique_ptr<Param>> inputs;
  std::list<std::unique_ptr<Param>> outputs;

 public:
  OWSParameter() = default;
  OWSParameter(const OWSParameter &) = delete;
  OWSParameter(OWSParameter &&) = delete;

  ~OWSParameter() override{
    std::cout  << "DELETEDELETEDELETEDELETEDELETEDELETEDELETEDELETEDELETE\n";
  }
};
}  // namespace EOEPCA::OWS
#endif  // EOEPCAOWS_OWSPARAMETER_HPP
