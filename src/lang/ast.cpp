#include "ast.hpp"

namespace aura::lang {

void IntExpr::dump(std::ostream& os, int indent) const {
    os << std::string(indent, ' ') << "Int(" << value << ")\n";
}
void FloatExpr::dump(std::ostream& os, int indent) const {
    os << std::string(indent, ' ') << "Float(" << value << ")\n";
}
void BoolExpr::dump(std::ostream& os, int indent) const {
    os << std::string(indent, ' ') << "Bool(" << (value ? "true" : "false") << ")\n";
}
void IdentExpr::dump(std::ostream& os, int indent) const {
    os << std::string(indent, ' ') << "Ident(" << name << ")\n";
}
void CallExpr::dump(std::ostream& os, int indent) const {
    os << std::string(indent, ' ') << "Call(" << name << "):\n";
    for (const auto& a : args) a->dump(os, indent + 2);
}
void UnaryExpr::dump(std::ostream& os, int indent) const {
    os << std::string(indent, ' ') << "Unary(" << tok_name(op) << "):\n";
    expr->dump(os, indent + 2);
}
void BinaryExpr::dump(std::ostream& os, int indent) const {
    os << std::string(indent, ' ') << "Binary(" << tok_name(op) << "):\n";
    left->dump(os, indent + 2);
    right->dump(os, indent + 2);
}

void Strategy::dump(std::ostream& os) const {
    os << "Strategy(" << name << "):\n";
    os << "  Params:\n";
    for (const auto& p : params) {
        os << "    " << p.name << " : " << tok_name(p.type) << " = ";
        p.init->dump(os, 0); // dump automatski dodaje \n
    }
    os << "  Lets:\n";
    for (const auto& l : lets) {
        os << "    let " << l.name << " =\n";
        l.expr->dump(os, 6);
    }
    os << "  Rules:\n";
    for (const auto& r : rules) {
        os << "    " << (r.is_enter ? "enter" : "exit") << " "
           << (r.is_long ? "long" : "short") << " when:\n";
        r.cond->dump(os, 6);
    }
    if (size) {
        os << "  Size:\n";
        size->dump(os, 4);
    }
}

} // namespace aura::lang