%language "c++"
%define api.value.type variant
%define api.token.constructor
%define parse.error detailed
%define api.parser.class { ParserImpl }
%locations

%code requires {
    #include "lang/ast.hpp"
    #include "lang/lexer.hpp"
    #include <memory>
    #include <vector>
    #include <string>
}

%code {
    #include "lang/parser.hpp"
    #include <stdexcept>
    #include <string>

    // Bison podrazumevano stavlja ParserImpl u namespace 'yy'.
    using ParserImpl = yy::ParserImpl;

    static aura::lang::SourceLocation to_src_loc(const ParserImpl::location_type& loc) {
        return {static_cast<uint32_t>(loc.begin.line),
                static_cast<uint32_t>(loc.begin.column)};
    }

    static ParserImpl::location_type to_yy_loc(aura::lang::SourceLocation loc) {
        ParserImpl::location_type l;
        l.begin.line = loc.line;
        l.begin.column = loc.col;
        l.end = l.begin;
        return l;
    }

    static ParserImpl::symbol_type yylex(aura::lang::Lexer& lex) {
        using namespace aura::lang;
        Token t = lex.next();
        auto loc = to_yy_loc(t.loc);
        switch (t.kind) {
            case Tok::END:          return ParserImpl::make_END(loc);
            case Tok::KW_STRATEGY:  return ParserImpl::make_KW_STRATEGY(loc);
            case Tok::KW_PARAM:     return ParserImpl::make_KW_PARAM(loc);
            case Tok::KW_LET:       return ParserImpl::make_KW_LET(loc);
            case Tok::KW_ENTER:     return ParserImpl::make_KW_ENTER(loc);
            case Tok::KW_EXIT:      return ParserImpl::make_KW_EXIT(loc);
            case Tok::KW_LONG:      return ParserImpl::make_KW_LONG(loc);
            case Tok::KW_SHORT:     return ParserImpl::make_KW_SHORT(loc);
            case Tok::KW_WHEN:      return ParserImpl::make_KW_WHEN(loc);
            case Tok::KW_SIZE:      return ParserImpl::make_KW_SIZE(loc);
            case Tok::KW_TRUE:      return ParserImpl::make_KW_TRUE(loc);
            case Tok::KW_FALSE:     return ParserImpl::make_KW_FALSE(loc);
            case Tok::KW_INT:       return ParserImpl::make_KW_INT(loc);
            case Tok::KW_FLOAT:     return ParserImpl::make_KW_FLOAT(loc);
            case Tok::KW_BOOL:      return ParserImpl::make_KW_BOOL(loc);
            case Tok::INT_LIT:      return ParserImpl::make_INT_LIT(t.int_val, loc);
            case Tok::FLOAT_LIT:    return ParserImpl::make_FLOAT_LIT(t.float_val, loc);
            case Tok::IDENT:        return ParserImpl::make_IDENT(t.ident, loc);
            case Tok::PLUS:         return ParserImpl::make_PLUS(loc);
            case Tok::MINUS:        return ParserImpl::make_MINUS(loc);
            case Tok::STAR:         return ParserImpl::make_STAR(loc);
            case Tok::SLASH:        return ParserImpl::make_SLASH(loc);
            case Tok::PERCENT:      return ParserImpl::make_PERCENT(loc);
            case Tok::LT:           return ParserImpl::make_LT(loc);
            case Tok::LE:           return ParserImpl::make_LE(loc);
            case Tok::GT:           return ParserImpl::make_GT(loc);
            case Tok::GE:           return ParserImpl::make_GE(loc);
            case Tok::EQ:           return ParserImpl::make_EQ(loc);
            case Tok::NE:           return ParserImpl::make_NE(loc);
            case Tok::AND:          return ParserImpl::make_AND(loc);
            case Tok::OR:           return ParserImpl::make_OR(loc);
            case Tok::NOT:          return ParserImpl::make_NOT(loc);
            case Tok::ASSIGN:       return ParserImpl::make_ASSIGN(loc);
            case Tok::LPAREN:       return ParserImpl::make_LPAREN(loc);
            case Tok::RPAREN:       return ParserImpl::make_RPAREN(loc);
            case Tok::LBRACE:       return ParserImpl::make_LBRACE(loc);
            case Tok::RBRACE:       return ParserImpl::make_RBRACE(loc);
            case Tok::COMMA:        return ParserImpl::make_COMMA(loc);
            case Tok::SEMI:         return ParserImpl::make_SEMI(loc);
            case Tok::COLON:        return ParserImpl::make_COLON(loc);
            case Tok::LEX_ERROR:    throw std::runtime_error("lexical error at " + std::to_string(t.loc.line) + ":" + std::to_string(t.loc.col));
            default:                return ParserImpl::make_END(loc);
        }
    }

    static aura::lang::ExprPtr make_binary(aura::lang::SourceLocation loc, aura::lang::Tok op, aura::lang::ExprPtr l, aura::lang::ExprPtr r) {
        auto e = std::make_unique<aura::lang::BinaryExpr>();
        e->loc = loc; e->op = op; e->left = std::move(l); e->right = std::move(r);
        return e;
    }
    static aura::lang::ExprPtr make_unary(aura::lang::SourceLocation loc, aura::lang::Tok op, aura::lang::ExprPtr e) {
        auto u = std::make_unique<aura::lang::UnaryExpr>();
        u->loc = loc; u->op = op; u->expr = std::move(e);
        return u;
    }

    void ParserImpl::error(const location_type& loc, const std::string& msg) {
        errors_.push_back(std::to_string(loc.begin.line) + ":" + std::to_string(loc.begin.column) + ": error: " + msg);
    }

    namespace aura::lang {
        Parser::Parser(std::istream& in) : lexer_(&in) {}
        std::unique_ptr<Strategy> Parser::parse() {
            std::unique_ptr<Strategy> result;
            ParserImpl p(lexer_, errors_, result);
            if (p.parse() != 0) return nullptr;
            return result;
        }
    }
}

%param { aura::lang::Lexer& lex }
%parse-param { std::vector<std::string>& errors_ }
%parse-param { std::unique_ptr<aura::lang::Strategy>& result }

%token END 0 "end of file"
%token KW_STRATEGY "strategy" KW_PARAM "param" KW_LET "let"
%token KW_ENTER "enter" KW_EXIT "exit" KW_LONG "long" KW_SHORT "short" KW_WHEN "when"
%token KW_SIZE "size" KW_TRUE "true" KW_FALSE "false"
%token KW_INT "int" KW_FLOAT "float" KW_BOOL "bool"

%token <int64_t> INT_LIT "integer literal"
%token <double> FLOAT_LIT "float literal"
%token <std::string> IDENT "identifier"

%token PLUS "+" MINUS "-" STAR "*" SLASH "/" PERCENT "%"
%token LT "<" LE "<=" GT ">" GE ">=" EQ "==" NE "!="
%token AND "&&" OR "||" NOT "!" ASSIGN "="
%token LPAREN "(" RPAREN ")" LBRACE "{" RBRACE "}"
%token COMMA "," SEMI ";" COLON ":"

%left OR
%left AND
%nonassoc LT LE GT GE EQ NE
%left PLUS MINUS
%left STAR SLASH PERCENT
%right UMINUS NOT

%type <aura::lang::ExprPtr> expr or_expr and_expr cmp_expr add_expr mul_expr unary_expr primary
%type <aura::lang::ExprPtr> size_opt
%type <std::vector<aura::lang::ExprPtr>> arg_list
%type <std::vector<aura::lang::ParamDecl>> param_list
%type <aura::lang::ParamDecl> param_decl
%type <std::vector<aura::lang::LetDecl>> let_list
%type <aura::lang::LetDecl> let_decl
%type <std::vector<aura::lang::RuleDecl>> rule_list
%type <aura::lang::RuleDecl> rule_decl
%type <aura::lang::Strategy> strategy

%%

program:
    strategy { result = std::make_unique<aura::lang::Strategy>(std::move($1)); }
    ;

strategy:
    KW_STRATEGY IDENT LBRACE param_list let_list rule_list size_opt RBRACE {
        $$.name = $2;
        $$.params = std::move($4);
        $$.lets = std::move($5);
        $$.rules = std::move($6);
        $$.size = std::move($7);
        $$.loc = to_src_loc(@1);
    }
    | KW_STRATEGY error RBRACE { yyerrok; }
    ;

param_list:
    /* empty */ { $$ = std::vector<aura::lang::ParamDecl>(); }
    | param_list param_decl { $$ = std::move($1); $$.push_back(std::move($2)); }
    ;

param_decl:
    KW_PARAM IDENT COLON KW_INT ASSIGN expr SEMI {
        $$.name = $2; $$.type = aura::lang::Tok::KW_INT; $$.init = std::move($6); $$.loc = to_src_loc(@1);
    }
    | KW_PARAM IDENT COLON KW_FLOAT ASSIGN expr SEMI {
        $$.name = $2; $$.type = aura::lang::Tok::KW_FLOAT; $$.init = std::move($6); $$.loc = to_src_loc(@1);
    }
    | KW_PARAM error SEMI { yyerrok; }
    ;

let_list:
    /* empty */ { $$ = std::vector<aura::lang::LetDecl>(); }
    | let_list let_decl { $$ = std::move($1); $$.push_back(std::move($2)); }
    ;

let_decl:
    KW_LET IDENT ASSIGN expr SEMI {
        $$.name = $2; $$.expr = std::move($4); $$.loc = to_src_loc(@1);
    }
    | KW_LET error SEMI { yyerrok; }
    ;

rule_list:
    /* empty */ { $$ = std::vector<aura::lang::RuleDecl>(); }
    | rule_list rule_decl { $$ = std::move($1); $$.push_back(std::move($2)); }
    ;

rule_decl:
    KW_ENTER KW_LONG KW_WHEN expr SEMI { $$.is_enter = true; $$.is_long = true; $$.cond = std::move($4); $$.loc = to_src_loc(@1); }
    | KW_ENTER KW_SHORT KW_WHEN expr SEMI { $$.is_enter = true; $$.is_long = false; $$.cond = std::move($4); $$.loc = to_src_loc(@1); }
    | KW_EXIT KW_LONG KW_WHEN expr SEMI { $$.is_enter = false; $$.is_long = true; $$.cond = std::move($4); $$.loc = to_src_loc(@1); }
    | KW_EXIT KW_SHORT KW_WHEN expr SEMI { $$.is_enter = false; $$.is_long = false; $$.cond = std::move($4); $$.loc = to_src_loc(@1); }
    | KW_ENTER error SEMI { yyerrok; }
    | KW_EXIT  error SEMI { yyerrok; }
    ;

size_opt:
    /* empty */ { $$ = nullptr; }
    | KW_SIZE ASSIGN expr SEMI { $$ = std::move($3); }
    ;

expr:
    or_expr { $$ = std::move($1); }
    ;

or_expr:
    and_expr { $$ = std::move($1); }
    | or_expr OR and_expr { $$ = make_binary(to_src_loc(@2), aura::lang::Tok::OR, std::move($1), std::move($3)); }
    ;

and_expr:
    cmp_expr { $$ = std::move($1); }
    | and_expr AND cmp_expr { $$ = make_binary(to_src_loc(@2), aura::lang::Tok::AND, std::move($1), std::move($3)); }
    ;

cmp_expr:
    add_expr { $$ = std::move($1); }
    | add_expr LT add_expr { $$ = make_binary(to_src_loc(@2), aura::lang::Tok::LT, std::move($1), std::move($3)); }
    | add_expr LE add_expr { $$ = make_binary(to_src_loc(@2), aura::lang::Tok::LE, std::move($1), std::move($3)); }
    | add_expr GT add_expr { $$ = make_binary(to_src_loc(@2), aura::lang::Tok::GT, std::move($1), std::move($3)); }
    | add_expr GE add_expr { $$ = make_binary(to_src_loc(@2), aura::lang::Tok::GE, std::move($1), std::move($3)); }
    | add_expr EQ add_expr { $$ = make_binary(to_src_loc(@2), aura::lang::Tok::EQ, std::move($1), std::move($3)); }
    | add_expr NE add_expr { $$ = make_binary(to_src_loc(@2), aura::lang::Tok::NE, std::move($1), std::move($3)); }
    ;

add_expr:
    mul_expr { $$ = std::move($1); }
    | add_expr PLUS mul_expr { $$ = make_binary(to_src_loc(@2), aura::lang::Tok::PLUS, std::move($1), std::move($3)); }
    | add_expr MINUS mul_expr { $$ = make_binary(to_src_loc(@2), aura::lang::Tok::MINUS, std::move($1), std::move($3)); }
    ;

mul_expr:
    unary_expr { $$ = std::move($1); }
    | mul_expr STAR unary_expr { $$ = make_binary(to_src_loc(@2), aura::lang::Tok::STAR, std::move($1), std::move($3)); }
    | mul_expr SLASH unary_expr { $$ = make_binary(to_src_loc(@2), aura::lang::Tok::SLASH, std::move($1), std::move($3)); }
    | mul_expr PERCENT unary_expr { $$ = make_binary(to_src_loc(@2), aura::lang::Tok::PERCENT, std::move($1), std::move($3)); }
    ;

unary_expr:
    primary { $$ = std::move($1); }
    | MINUS unary_expr %prec UMINUS { $$ = make_unary(to_src_loc(@1), aura::lang::Tok::MINUS, std::move($2)); }
    | NOT unary_expr { $$ = make_unary(to_src_loc(@1), aura::lang::Tok::NOT, std::move($2)); }
    ;

primary:
    INT_LIT { auto e = std::make_unique<aura::lang::IntExpr>(); e->loc = to_src_loc(@1); e->value = $1; $$ = std::move(e); }
    | FLOAT_LIT { auto e = std::make_unique<aura::lang::FloatExpr>(); e->loc = to_src_loc(@1); e->value = $1; $$ = std::move(e); }
    | KW_TRUE { auto e = std::make_unique<aura::lang::BoolExpr>(); e->loc = to_src_loc(@1); e->value = true; $$ = std::move(e); }
    | KW_FALSE { auto e = std::make_unique<aura::lang::BoolExpr>(); e->loc = to_src_loc(@1); e->value = false; $$ = std::move(e); }
    | IDENT { auto e = std::make_unique<aura::lang::IdentExpr>(); e->loc = to_src_loc(@1); e->name = $1; $$ = std::move(e); }
    | IDENT LPAREN arg_list RPAREN {
        auto e = std::make_unique<aura::lang::CallExpr>();
        e->loc = to_src_loc(@1); e->name = $1; e->args = std::move($3); $$ = std::move(e);
    }
    | LPAREN expr RPAREN { $$ = std::move($2); }
    ;

arg_list:
    /* empty */ { $$ = std::vector<aura::lang::ExprPtr>(); }
    | expr { $$ = std::vector<aura::lang::ExprPtr>(); $$.push_back(std::move($1)); }
    | arg_list COMMA expr { $$ = std::move($1); $$.push_back(std::move($3)); }
    ;

%%