#ifndef TOKEN_HPP
#define TOKEN_HPP
#include<cstdint>
#include <string>
namespace aura::lang{
    enum class Tok : uint16_t{
    END = 0,
    // Keywords
    KW_STRATEGY, KW_PARAM, KW_LET,
    KW_ENTER, KW_EXIT, KW_LONG, KW_SHORT, KW_WHEN,
    KW_SIZE, KW_TRUE, KW_FALSE,
    KW_INT, KW_FLOAT, KW_BOOL,
    // Literals
    INT_LIT, FLOAT_LIT, BOOL_LIT,
    IDENT,
    // Operators
    PLUS, MINUS, STAR, SLASH, PERCENT,
    LT, LE, GT, GE, EQ, NE,
    AND, OR, NOT, ASSIGN,
    // Separators
    LPAREN, RPAREN, LBRACE, RBRACE,
    COMMA, SEMI, COLON,
    // Error
    LEX_ERROR,
    };
    struct SourceLocation {
        uint32_t line = 1;
        uint32_t col  = 1;
    };

    struct Token {
        Tok            kind;
        SourceLocation loc;
        int64_t        int_val   = 0;
        double         float_val = 0.0;
        bool           bool_val  = false;
        std::string    ident;      // only for IDENT
    };

    const char* tok_name(Tok k);
}













#endif // TOKEN_HPP