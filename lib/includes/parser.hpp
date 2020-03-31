#ifndef EOEPCAOWS_PARSER_HPP
#define EOEPCAOWS_PARSER_HPP

#include <string>

namespace EOEPCA {
class Parser {
  std::string name{"EOEPCA OWS Parser "};

 public:
  Parser() = default;
  Parser(const Parser&) = default;
  Parser(Parser&&) = default;

  ~Parser();

  // Get & Set
 public:

  const std::string& getName() const;
};

}  // namespace EOEPCA

#endif  // EOEPCAOWS_PARSER_HPP
