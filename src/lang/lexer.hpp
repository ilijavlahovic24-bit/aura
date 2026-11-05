#ifndef AURA_LANG_LEXER_HPP
#define AURA_LANG_LEXER_HPP

#include "token.hpp"
#include <FlexLexer.h>
#include <istream>

namespace aura::lang {

class Lexer : public yyFlexLexer {
public:
    explicit Lexer(std::istream* in);

    Token next();      // glavni ulaz — vraća sledeći token
    SourceLocation loc() const { return loc_; }

private:
    SourceLocation loc_{1, 1};
    Token make(Tok kind) const;
    Token make_op(Tok kind) const;
};

} // namespace aura::lang

#endif