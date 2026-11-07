#include "lang/ir.hpp"

namespace aura::lang {

static const char* op_name(IROp op) {
    switch (op) {
        case IROp::ConstInt: return "ConstInt";
        case IROp::ConstFloat: return "ConstFloat";
        case IROp::ConstBool: return "ConstBool";
        case IROp::LoadSeries: return "LoadSeries";
        case IROp::LoadParam: return "LoadParam";
        case IROp::Neg: return "Neg";
        case IROp::Not: return "Not";
        case IROp::Add: return "Add";
        case IROp::Sub: return "Sub";
        case IROp::Mul: return "Mul";
        case IROp::Div: return "Div";
        case IROp::Mod: return "Mod";
        case IROp::Lt: return "Lt";
        case IROp::Le: return "Le";
        case IROp::Gt: return "Gt";
        case IROp::Ge: return "Ge";
        case IROp::Eq: return "Eq";
        case IROp::Ne: return "Ne";
        case IROp::And: return "And";
        case IROp::Or: return "Or";
        case IROp::Call: return "Call";
        case IROp::EnterLong: return "EnterLong";
        case IROp::ExitLong: return "ExitLong";
        case IROp::EnterShort: return "EnterShort";
        case IROp::ExitShort: return "ExitShort";
        case IROp::SetSize: return "SetSize";
    }
    return "?";
}

void IRModule::dump(std::ostream& os) const {
    for (const auto& i : insts) {
        os << "%" << i.dst << " = " << op_name(i.op) << " " << type_name(i.type);
        
        if (i.op == IROp::ConstInt) os << "(" << i.i_val << ")";
        else if (i.op == IROp::ConstFloat) os << "(" << i.f_val << ")";
        else if (i.op == IROp::ConstBool) os << "(" << (i.b_val ? "true" : "false") << ")";
        else if (i.op == IROp::LoadSeries || i.op == IROp::LoadParam || i.op == IROp::Call) os << " " << i.name;
        
        if (!i.args.empty()) {
            os << "(";
            for (size_t j = 0; j < i.args.size(); ++j) {
                if (j > 0) os << ", ";
                if (i.args[j] >= 0) os << "%" << i.args[j];
                else os << "const"; // Placeholder for constanst if there are any, but in this IR representation, 
                                    //constants are not directly referenced by index.
            }
            os << ")";
        }
        os << "\n";
    }
}

} // namespace aura::lang