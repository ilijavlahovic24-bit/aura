#include "token.hpp"
namespace aura::lang {

const char* tok_name(Tok k) {
    switch (k) {
        case Tok::END:          return "EOF";
        case Tok::KW_STRATEGY:  return "strategy";
        case Tok::KW_PARAM:     return "param";
        case Tok::KW_LET:       return "let";
        case Tok::KW_ENTER:     return "enter";
        case Tok::KW_EXIT:      return "exit";
        case Tok::KW_LONG:      return "long";
        case Tok::KW_SHORT:     return "short";
        case Tok::KW_WHEN:      return "when";
        case Tok::KW_SIZE:      return "size";
        case Tok::KW_TRUE:      return "true";
        case Tok::KW_FALSE:     return "false";
        case Tok::KW_INT:       return "int";
        case Tok::KW_FLOAT:     return "float";
        case Tok::KW_BOOL:      return "bool";
        case Tok::INT_LIT:      return "INT_LIT";
        case Tok::FLOAT_LIT:    return "FLOAT_LIT";
        case Tok::BOOL_LIT:     return "BOOL_LIT";
        case Tok::IDENT:        return "IDENT";
        case Tok::PLUS:         return "+";
        case Tok::MINUS:        return "-";
        case Tok::STAR:         return "*";
        case Tok::SLASH:        return "/";
        case Tok::PERCENT:      return "%";
        case Tok::LT:           return "<";
        case Tok::LE:           return "<=";
        case Tok::GT:           return ">";
        case Tok::GE:           return ">=";
        case Tok::EQ:           return "==";
        case Tok::NE:           return "!=";
        case Tok::AND:          return "&&";
        case Tok::OR:           return "||";
        case Tok::NOT:          return "!";
        case Tok::ASSIGN:       return "=";
        case Tok::LPAREN:       return "(";
        case Tok::RPAREN:       return ")";
        case Tok::LBRACE:       return "{";
        case Tok::RBRACE:       return "}";
        case Tok::COMMA:        return ",";
        case Tok::SEMI:         return ";";
        case Tok::COLON:        return ":";
        case Tok::LEX_ERROR:    return "LEX_ERROR";
    }
    return "?";
}

} // namespace aura::lang