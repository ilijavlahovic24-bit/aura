#ifndef AURA_LANG_AST_HPP
#define AURA_LANG_AST_HPP

#include "lang/token.hpp"
#include <memory>
#include <string>
#include <vector>
#include <iostream>

namespace aura::lang {

struct Expr {
    SourceLocation loc;
    virtual ~Expr() = default;
    virtual void dump(std::ostream& os, int indent = 0) const = 0;
};
using ExprPtr = std::unique_ptr<Expr>;

struct IntExpr : Expr {
    int64_t value;
    void dump(std::ostream& os, int indent = 0) const override;
};

struct FloatExpr : Expr {
    double value;
    void dump(std::ostream& os, int indent = 0) const override;
};

struct BoolExpr : Expr {
    bool value;
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
    Tok op;
    ExprPtr expr;
    void dump(std::ostream& os, int indent = 0) const override;
};

struct BinaryExpr : Expr {
    Tok op;
    ExprPtr left;
    ExprPtr right;
    void dump(std::ostream& os, int indent = 0) const override;
};

struct ParamDecl {
    std::string name;
    Tok type; // KW_INT ili KW_FLOAT
    ExprPtr init;
    SourceLocation loc;
};

struct LetDecl {
    std::string name;
    ExprPtr expr;
    SourceLocation loc;
};

struct RuleDecl {
    bool is_enter;
    bool is_long;
    ExprPtr cond;
    SourceLocation loc;
};

struct Strategy {
    std::string name;
    std::vector<ParamDecl> params;
    std::vector<LetDecl> lets;
    std::vector<RuleDecl> rules;
    ExprPtr size;
    SourceLocation loc;
    void dump(std::ostream& os) const;
};

} // namespace aura::lang

#endif