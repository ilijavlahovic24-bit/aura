#ifndef AURA_LANG_LEXER_HPP
#define AURA_LANG_LEXER_HPP

#include "token.hpp"
#if !defined(__FLEX_LEXER_H) && !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif
#include <istream>

namespace aura::lang {

class Lexer : public yyFlexLexer {
public:
    explicit Lexer(std::istream* in);

    Token next();        
    int yylex() override;
    SourceLocation loc() const { return loc_; }

private:
    SourceLocation loc_{1, 1};
    Token make(Tok kind) const;
    Token make_op(Tok kind) const;
};

} // namespace aura::lang

#endif