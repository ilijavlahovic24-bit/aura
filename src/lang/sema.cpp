#include "sema.hpp"
#include <cmath>
#include <iostream>

namespace aura::lang {

SemanticAnalyzer::SemanticAnalyzer() {
    symbols_["open"]   = Type::SeriesFloat;
    symbols_["high"]   = Type::SeriesFloat;
    symbols_["low"]    = Type::SeriesFloat;
    symbols_["close"]  = Type::SeriesFloat;
    symbols_["volume"] = Type::SeriesFloat;
}

void SemanticAnalyzer::error(SourceLocation loc, const std::string& msg) {
    errors_.push_back(filename_ + ":" + std::to_string(loc.line) + ":" + std::to_string(loc.col) + ": error: " + msg);
}

Type SemanticAnalyzer::check_binary(BinaryExpr* expr) {
    Type l = infer(expr->left.get());
    Type r = infer(expr->right.get());
    if (l == Type::Error || r == Type::Error) return Type::Error;

    bool is_cmp = (expr->op >= Tok::LT && expr->op <= Tok::NE);
    bool is_logic = (expr->op == Tok::AND || expr->op == Tok::OR);
    bool is_arith = (expr->op >= Tok::PLUS && expr->op <= Tok::PERCENT);

    if (is_logic) {
        if (!is_bool(l) || !is_bool(r)) {
            error(expr->loc, "logical operators require bool operands");
            return Type::Error;
        }
        return (is_series(l) || is_series(r)) ? Type::SeriesBool : Type::Bool;
    }

    if (is_cmp) {
        if (!is_numeric(l) || !is_numeric(r)) {
            error(expr->loc, "comparison requires numeric operands");
            return Type::Error;
        }
        return (is_series(l) || is_series(r)) ? Type::SeriesBool : Type::Bool;
    }

    if (is_arith) {
        if (!is_numeric(l) || !is_numeric(r)) {
            error(expr->loc, "arithmetic requires numeric operands");
            return Type::Error;
        }
        // Result is series if either operand is series
        if (is_series(l) || is_series(r)) return Type::SeriesFloat;
        // If both are scalars, result is int if both are int, otherwise float
        if (l == Type::Int && r == Type::Int) return Type::Int;
        return Type::Float;
    }

    return Type::Error;
}

Type SemanticAnalyzer::check_unary(UnaryExpr* expr) {
    Type t = infer(expr->expr.get());
    if (t == Type::Error) return Type::Error;

    if (expr->op == Tok::MINUS) {
        if (!is_numeric(t)) {
            error(expr->loc, "unary minus requires numeric operand");
            return Type::Error;
        }
        return t;
    }
    if (expr->op == Tok::NOT) {
        if (!is_bool(t)) {
            error(expr->loc, "operator '!' requires bool operand");
            return Type::Error;
        }
        return t;
    }
    return Type::Error;
}

Type SemanticAnalyzer::check_call(CallExpr* expr) {
    const std::string& fn = expr->name;
    
    struct Builtin { int min_args; int max_args; bool needs_const_window; Type ret; };
    static const std::unordered_map<std::string, Builtin> builtins = {
        {"sma",         {2, 2, true,  Type::SeriesFloat}},
        {"ema",         {2, 2, true,  Type::SeriesFloat}},
        {"stdev",       {2, 2, true,  Type::SeriesFloat}},
        {"rsi",         {2, 2, true,  Type::SeriesFloat}},
        {"atr",         {1, 1, true,  Type::SeriesFloat}},
        {"highest",     {2, 2, true,  Type::SeriesFloat}},
        {"lowest",      {2, 2, true,  Type::SeriesFloat}},
        {"shift",       {2, 2, true,  Type::SeriesFloat}},
        {"crossover",   {2, 2, false, Type::SeriesBool}},
        {"crossunder",  {2, 2, false, Type::SeriesBool}},
        {"abs",         {1, 1, false, Type::SeriesFloat}},
        {"sqrt",        {1, 1, false, Type::SeriesFloat}},
        {"log",         {1, 1, false, Type::SeriesFloat}},
        {"min",         {2, 2, false, Type::SeriesFloat}},
        {"max",         {2, 2, false, Type::SeriesFloat}},
        {"where",       {3, 3, false, Type::SeriesFloat}}
    };

    auto it = builtins.find(fn);
    if (it == builtins.end()) {
        error(expr->loc, "unknown function '" + fn + "'");
        return Type::Error;
    }

    const auto& b = it->second;
    if ((int)expr->args.size() < b.min_args || (int)expr->args.size() > b.max_args) {
        error(expr->loc, "function '" + fn + "' expects " + std::to_string(b.min_args) + " arguments");
        return Type::Error;
    }

    for (size_t i = 0; i < expr->args.size(); ++i) {
        Type arg_t = infer(expr->args[i].get());
        if (arg_t == Type::Error) return Type::Error;

        // Provera za konstantni prozor (npr. lookback)
        if (b.needs_const_window && i == 1) {
            if (is_series(arg_t)) {
                error(expr->args[i]->loc, "window argument must be a constant or scalar");
            }
        }
    }
    // Specific rules for functions
    if (fn == "where") {
        Type c = expr->args[0]->type;
        Type a = expr->args[1]->type;
        Type b = expr->args[2]->type;
        if (!is_bool(c)) error(expr->args[0]->loc, "where condition must be bool");
        if (a != b) error(expr->args[1]->loc, "where branches must have the same type");
        return is_series(a) || is_series(c) ? to_series(a) : a;
    }

    if (fn == "crossover" || fn == "crossunder" || fn == "min" || fn == "max") {
        Type a = expr->args[0]->type;
        Type b = expr->args[1]->type;
        if (!is_numeric(a) || !is_numeric(b)) error(expr->loc, fn + " requires numeric arguments");
        return Type::SeriesFloat; // Always returns series<float> for these functions
    }

    return b.ret;
}

Type SemanticAnalyzer::infer(Expr* expr) {
    if (!expr) return Type::Error;
    
    if (auto e = dynamic_cast<IntExpr*>(expr)) { e->type = Type::Int; return e->type; }
    if (auto e = dynamic_cast<FloatExpr*>(expr)) { e->type = Type::Float; return e->type; }
    if (auto e = dynamic_cast<BoolExpr*>(expr)) { e->type = Type::Bool; return e->type; }
    
    if (auto e = dynamic_cast<IdentExpr*>(expr)) {
        auto it = symbols_.find(e->name);
        if (it == symbols_.end()) {
            error(e->loc, "undefined variable '" + e->name + "'");
            e->type = Type::Error;
        } else {
            e->type = it->second;
        }
        return e->type;
    }

    if (auto e = dynamic_cast<BinaryExpr*>(expr)) { e->type = check_binary(e); return e->type; }
    if (auto e = dynamic_cast<UnaryExpr*>(expr)) { e->type = check_unary(e); return e->type; }
    if (auto e = dynamic_cast<CallExpr*>(expr)) { e->type = check_call(e); return e->type; }

    return Type::Error;
}

void SemanticAnalyzer::validate_rules(const Strategy& strat) {
    bool has_enter_long = false, has_exit_long = false;
    bool has_enter_short = false, has_exit_short = false;

    for (const auto& rule : strat.rules) {
        Type t = infer(rule.cond.get());
        if (t != Type::Bool && t != Type::SeriesBool && t != Type::Error) {
            error(rule.cond->loc, "rule condition must be bool or series<bool>");
        }
        if (rule.is_enter && rule.is_long) has_enter_long = true;
        if (!rule.is_enter && rule.is_long) has_exit_long = true;
        if (rule.is_enter && !rule.is_long) has_enter_short = true;
        if (!rule.is_enter && !rule.is_long) has_exit_short = true;
    }

    if (!has_enter_long && !has_enter_short) {
        error(strat.loc, "strategy must have at least one enter rule");
    }
    if (has_enter_long && !has_exit_long) {
        error(strat.loc, "missing exit long rule for enter long");
    }
    if (has_enter_short && !has_exit_short) {
        error(strat.loc, "missing exit short rule for enter short");
    }
}

void SemanticAnalyzer::validate_size(const Strategy& strat) {
    if (strat.size) {
        Type t = infer(strat.size.get());
        if (t != Type::Float && t != Type::SeriesFloat && t != Type::Error) {
            error(strat.size->loc, "size must be float or series<float>");
        }
    }
}

bool SemanticAnalyzer::analyze(Strategy& strat, const std::string& filename) {
    filename_ = filename;
    symbols_.clear();
    errors_.clear();
    // 1. Define predefined series
    symbols_["open"]   = Type::SeriesFloat;
    symbols_["high"]   = Type::SeriesFloat;
    symbols_["low"]    = Type::SeriesFloat;
    symbols_["close"]  = Type::SeriesFloat;
    symbols_["volume"] = Type::SeriesFloat;
    // 2. Define parameters
    for (auto& p : strat.params) {
        if (symbols_.count(p.name)) {
            error(p.loc, "redefinition of '" + p.name + "'");
            continue;
        }
        Type t = infer(p.init.get());
        if (p.type == Tok::KW_INT && t != Type::Int) error(p.loc, "param '" + p.name + "' must be int");
        if (p.type == Tok::KW_FLOAT && t != Type::Float && t != Type::Int) error(p.loc, "param '" + p.name + "' must be float");
        
        symbols_[p.name] = (p.type == Tok::KW_INT) ? Type::Int : Type::Float;
    }
    // 3. Define let declarations
    for (auto& l : strat.lets) {
        if (symbols_.count(l.name)) {
            error(l.loc, "redefinition of '" + l.name + "'");
        }
        Type t = infer(l.expr.get());
        symbols_[l.name] = t;
    }
    // 4. Validation of  rules
    validate_rules(strat);
    validate_size(strat);

    return errors_.empty();
}

} // namespace aura::lang