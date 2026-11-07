#ifndef AURA_LANG_IR_BUILDER_HPP
#define AURA_LANG_IR_BUILDER_HPP

#include "lang/ast.hpp"
#include "lang/ir.hpp"
#include <unordered_map>

namespace aura::lang {

class IRBuilder {
public:
    IRModule build(const Strategy& strat);

private:
    IRModule mod_;
    std::unordered_map<std::string, int> vars_; // let -> VReg

    int visit(Expr* expr);
};

} // namespace aura::lang

#endif