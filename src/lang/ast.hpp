#ifndef AURA_LANG_AST_HPP
#define AURA_LANG_AST_HPP

#include "lang/token.hpp"
#include "type.hpp"
#include <memory>
#include <string>
#include <vector>
#include <iostream>

namespace aura::lang {

// Base node for all expressions.
// The 'type' field is filled in the semantic analysis (sema.cpp).
// Initial is Error, which means "not yet calculated".
struct Expr {
    SourceLocation loc;
    Type type = Type::Error;
    virtual ~Expr() = default;
    virtual void dump(std::ostream& os, int indent = 0) const = 0;
};
using ExprPtr = std::unique_ptr<Expr>;

struct IntExpr : Expr {
    int64_t value = 0;
    void dump(std::ostream& os, int indent = 0) const override;
};

struct FloatExpr : Expr {
    double value = 0.0;
    void dump(std::ostream& os, int indent = 0) const override;
};

struct BoolExpr : Expr {
    bool value = false;
    void dump(std::ostream& os, int indent = 0) const override;
};

struct IdentExpr : Expr {
    std::string name;
    void dump(std::ostream& os, int indent = 0) const override;
};

struct CallExpr : Expr {
    std::string name;
    std::vector<ExprPtr> args;
    void dump(std::ostream& os, int indent = 0) const override;
};

struct UnaryExpr : Expr {
    Tok op = Tok::END;
    ExprPtr expr;
    void dump(std::ostream& os, int indent = 0) const override;
};

struct BinaryExpr : Expr {
    Tok op = Tok::END;
    ExprPtr left;
    ExprPtr right;
    void dump(std::ostream& os, int indent = 0) const override;
};

// ---- Declaration (top-level) ----

struct ParamDecl {
    std::string name;
    Tok type = Tok::END;  // KW_INT or KW_FLOAT
    ExprPtr init;
    SourceLocation loc;
};

struct LetDecl {
    std::string name;
    ExprPtr expr;
    SourceLocation loc;
};

struct RuleDecl {
    bool is_enter = false;
    bool is_long  = false;
    ExprPtr cond;
    SourceLocation loc;
};

struct Strategy {
    std::string name;
    std::vector<ParamDecl> params;
    std::vector<LetDecl>   lets;
    std::vector<RuleDecl>  rules;
    ExprPtr size;           // nullptr if size is not specified
    SourceLocation loc;
    void dump(std::ostream& os) const;
};

} // namespace aura::lang

#endif // AURA_LANG_AST_HPP