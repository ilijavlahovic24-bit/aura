#ifndef AURA_LANG_IR_HPP
#define AURA_LANG_IR_HPP

#include "lang/type.hpp"
#include <string>
#include <vector>
#include <iostream>

namespace aura::lang {

enum class IROp : uint8_t {
    ConstInt, ConstFloat, ConstBool,
    LoadSeries,     // open, high, low, close, volume
    LoadParam,      
    Neg, Not,
    Add, Sub, Mul, Div, Mod,
    Lt, Le, Gt, Ge, Eq, Ne,
    And, Or,
    Call,           // sma, ema, stdev, ...
    EnterLong, ExitLong,
    EnterShort, ExitShort,
    SetSize
};

struct IRInst {
    IROp op;
    Type type;
    int dst = -1;
    
    // Za args (used for Add, Call, etc.)
    std::vector<int> args; 
    
    // For consdtants
    int64_t i_val = 0;
    double f_val = 0.0;
    bool b_val = false;
    
    // For LoadSeries, LoadParam, Call
    std::string name; 
};

class IRModule {
public:
    std::vector<IRInst> insts;
    int next_reg = 0;

    int emit(IROp op, Type type, std::vector<int> args = {}) {
        IRInst inst;
        inst.op = op;
        inst.type = type;
        inst.dst = next_reg++;
        inst.args = std::move(args);
        insts.push_back(inst);
        return inst.dst;
    }

    void dump(std::ostream& os) const;
};

} // namespace aura::lang

#endif