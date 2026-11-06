#ifndef AURA_LANG_SEMA_HPP
#define AURA_LANG_SEMA_HPP

#include "ast.hpp"
#include "type.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace aura::lang {

class SemanticAnalyzer {
public:
    SemanticAnalyzer();
    //Returns true if analysis is successful, false if there were errors.
    bool analyze(Strategy& strat, const std::string& filename);
    
    const std::vector<std::string>& errors() const { return errors_; }

private:
    std::unordered_map<std::string, Type> symbols_;
    std::vector<std::string> errors_;
    std::string filename_;

    void error(SourceLocation loc, const std::string& msg);
    
    Type infer(Expr* expr);
    Type check_binary(BinaryExpr* expr);
    Type check_call(CallExpr* expr);
    Type check_unary(UnaryExpr* expr);

    // Provere specifične za strategiju
    void validate_rules(const Strategy& strat);
    void validate_size(const Strategy& strat);
};

} // namespace aura::lang

#endif