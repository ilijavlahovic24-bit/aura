#include "ir_builder.hpp"
#include <stdexcept>

namespace aura::lang {

int IRBuilder::visit(Expr* expr) {
    if (auto e = dynamic_cast<IntExpr*>(expr)) {
        int r = mod_.emit(IROp::ConstInt, Type::Int);
        mod_.insts.back().i_val = e->value;
        return r;
    }
    if (auto e = dynamic_cast<FloatExpr*>(expr)) {
        int r = mod_.emit(IROp::ConstFloat, Type::Float);
        mod_.insts.back().f_val = e->value;
        return r;
    }
    if (auto e = dynamic_cast<BoolExpr*>(expr)) {
        int r = mod_.emit(IROp::ConstBool, Type::Bool);
        mod_.insts.back().b_val = e->value;
        return r;
    }
    if (auto e = dynamic_cast<IdentExpr*>(expr)) {
        if (vars_.count(e->name)) return vars_[e->name];
        // If not a let, then a predefined series or parameter 
        // (Semantic analysis already checked if it exists)
        int r = mod_.emit(IROp::LoadSeries, Type::SeriesFloat);
        mod_.insts.back().name = e->name;
        return r;
    }
    if (auto e = dynamic_cast<UnaryExpr*>(expr)) {
        int v = visit(e->expr.get());
        IROp op = (e->op == Tok::MINUS) ? IROp::Neg : IROp::Not;
        return mod_.emit(op, e->type, {v});
    }
    if (auto e = dynamic_cast<BinaryExpr*>(expr)) {
        int l = visit(e->left.get());
        int r = visit(e->right.get());
        
        IROp op;
        switch (e->op) {
            case Tok::PLUS: op = IROp::Add; break;
            case Tok::MINUS: op = IROp::Sub; break;
            case Tok::STAR: op = IROp::Mul; break;
            case Tok::SLASH: op = IROp::Div; break;
            case Tok::PERCENT: op = IROp::Mod; break;
            case Tok::LT: op = IROp::Lt; break;
            case Tok::LE: op = IROp::Le; break;
            case Tok::GT: op = IROp::Gt; break;
            case Tok::GE: op = IROp::Ge; break;
            case Tok::EQ: op = IROp::Eq; break;
            case Tok::NE: op = IROp::Ne; break;
            case Tok::AND: op = IROp::And; break;
            case Tok::OR: op = IROp::Or; break;
            default: throw std::runtime_error("unknown binary op");
        }
        return mod_.emit(op, e->type, {l, r});
    }
    if (auto e = dynamic_cast<CallExpr*>(expr)) {
        std::vector<int> args;
        for (const auto& a : e->args) args.push_back(visit(a.get()));
        int r = mod_.emit(IROp::Call, e->type, std::move(args));
        mod_.insts.back().name = e->name;
        return r;
    }
    throw std::runtime_error("unknown expr");
}

IRModule IRBuilder::build(const Strategy& strat) {
    // 1. Parametar becomes LoadParam
    for (const auto& p : strat.params) {
        int r = mod_.emit(IROp::LoadParam, (p.type == Tok::KW_INT) ? Type::Int : Type::Float);
        mod_.insts.back().name = p.name;
        vars_[p.name] = r;
    }

    // 2. Let declarations
    for (const auto& l : strat.lets) {
        int r = visit(l.expr.get());
        vars_[l.name] = r;
    }

    // 3. Rules
    for (const auto& rule : strat.rules) {
        int cond = visit(rule.cond.get());
        IROp op;
        if (rule.is_enter && rule.is_long) op = IROp::EnterLong;
        else if (!rule.is_enter && rule.is_long) op = IROp::ExitLong;
        else if (rule.is_enter && !rule.is_long) op = IROp::EnterShort;
        else op = IROp::ExitShort;
        
        mod_.emit(op, Type::Void, {cond});
    }

    // 4. Size (opcional)
    if (strat.size) {
        int s = visit(strat.size.get());
        mod_.emit(IROp::SetSize, Type::Void, {s});
    } else {
        int s = mod_.emit(IROp::ConstFloat, Type::Float);
        mod_.insts.back().f_val = 1.0;
        mod_.emit(IROp::SetSize, Type::Void, {s});
    }

    return std::move(mod_);
}

} // namespace aura::lang