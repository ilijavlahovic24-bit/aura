%option c++ noyywrap nounput noinput
%option yyclass="aura::lang::Lexer"

%{
#include "lang/lexer.hpp"
#include <unordered_map>
#include <cstdlib>
#include <string>

#undef YY_DECL
#define YY_DECL aura::lang::Token aura::lang::Lexer::next()

using namespace aura::lang;

static const std::unordered_map<std::string, Tok> kKeywords = {
    {"strategy", Tok::KW_STRATEGY},
    {"param",    Tok::KW_PARAM},
    {"let",      Tok::KW_LET},
    {"enter",    Tok::KW_ENTER},
    {"exit",     Tok::KW_EXIT},
    {"long",     Tok::KW_LONG},
    {"short",    Tok::KW_SHORT},
    {"when",     Tok::KW_WHEN},
    {"size",     Tok::KW_SIZE},
    {"true",     Tok::KW_TRUE},
    {"false",    Tok::KW_FALSE},
    {"int",      Tok::KW_INT},
    {"float",    Tok::KW_FLOAT},
    {"bool",     Tok::KW_BOOL},
};
%}

%%

[ \t\r]+          { loc_.col += static_cast<uint32_t>(yyleng); }
\n                { ++loc_.line; loc_.col = 1; }
"//".*            { loc_.col += static_cast<uint32_t>(yyleng); }

[0-9]+\.[0-9]*([eE][+-]?[0-9]+)? {
    Token t = make(Tok::FLOAT_LIT);
    t.float_val = std::strtod(yytext, nullptr);
    loc_.col += static_cast<uint32_t>(yyleng);
    return t;
}
[0-9]+[eE][+-]?[0-9]+ {
    Token t = make(Tok::FLOAT_LIT);
    t.float_val = std::strtod(yytext, nullptr);
    loc_.col += static_cast<uint32_t>(yyleng);
    return t;
}
[0-9]+ {
    Token t = make(Tok::INT_LIT);
    t.int_val = std::strtoll(yytext, nullptr, 10);
    loc_.col += static_cast<uint32_t>(yyleng);
    return t;
}
[A-Za-z_][A-Za-z0-9_]* {
    Token t = make(Tok::IDENT);
    auto it = kKeywords.find(yytext);
    if (it != kKeywords.end()) {
        t.kind = it->second;
    } else {
        t.ident = yytext;
    }
    loc_.col += static_cast<uint32_t>(yyleng);
    return t;
}

"<=" { Token t = make_op(Tok::LE); loc_.col += 2; return t; }
">=" { Token t = make_op(Tok::GE); loc_.col += 2; return t; }
"==" { Token t = make_op(Tok::EQ); loc_.col += 2; return t; }
"!=" { Token t = make_op(Tok::NE); loc_.col += 2; return t; }
"&&" { Token t = make_op(Tok::AND); loc_.col += 2; return t; }
"||" { Token t = make_op(Tok::OR);  loc_.col += 2; return t; }

"+" { Token t = make_op(Tok::PLUS);    loc_.col += 1; return t; }
"-" { Token t = make_op(Tok::MINUS);   loc_.col += 1; return t; }
"*" { Token t = make_op(Tok::STAR);    loc_.col += 1; return t; }
"/" { Token t = make_op(Tok::SLASH);   loc_.col += 1; return t; }
"%" { Token t = make_op(Tok::PERCENT); loc_.col += 1; return t; }
"<" { Token t = make_op(Tok::LT);      loc_.col += 1; return t; }
">" { Token t = make_op(Tok::GT);      loc_.col += 1; return t; }
"!" { Token t = make_op(Tok::NOT);     loc_.col += 1; return t; }
"=" { Token t = make_op(Tok::ASSIGN);  loc_.col += 1; return t; }
"(" { Token t = make_op(Tok::LPAREN);  loc_.col += 1; return t; }
")" { Token t = make_op(Tok::RPAREN);  loc_.col += 1; return t; }
"{" { Token t = make_op(Tok::LBRACE);  loc_.col += 1; return t; }
"}" { Token t = make_op(Tok::RBRACE);  loc_.col += 1; return t; }
"," { Token t = make_op(Tok::COMMA);   loc_.col += 1; return t; }
";" { Token t = make_op(Tok::SEMI);    loc_.col += 1; return t; }
":" { Token t = make_op(Tok::COLON);   loc_.col += 1; return t; }

. {
    Token t = make(Tok::LEX_ERROR);
    loc_.col += static_cast<uint32_t>(yyleng);
    return t;
}

<<EOF>> { return make(Tok::END); }

%%

namespace aura::lang {

Lexer::Lexer(std::istream* in) : yyFlexLexer(in) {}

// <FlexLexer.h> deklarise "virtual int yylex();"
// Posto je YY_DECL override-ovan, flex ne generise yyFlexLexer::yylex(),
// pa ga moramo sami definisati da bi vtable bio kompletan.
int Lexer::yylex() {
    return 0;
}

Token Lexer::make(Tok kind) const {
    Token t;
    t.kind = kind;
    t.loc  = loc_;
    return t;
}

Token Lexer::make_op(Tok kind) const {
    return make(kind);
}

} // namespace aura::lang