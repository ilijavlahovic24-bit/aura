#ifndef AURA_LANG_PARSER_HPP
#define AURA_LANG_PARSER_HPP

#include "ast.hpp"
#include "lexer.hpp"
#include <istream>
#include <memory>
#include <string>
#include <vector>

namespace aura::lang {

class Parser {
public:
    explicit Parser(std::istream& in);
    std::unique_ptr<Strategy> parse();
    const std::vector<std::string>& errors() const { return errors_; }

private:
    Lexer lexer_;
    std::vector<std::string> errors_;
};

} // namespace aura::lang

#endif