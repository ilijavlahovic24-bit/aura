#include "lang/type.hpp"

namespace aura::lang {

const char* type_name(Type t) {
    switch (t) {
        case Type::Error:       return "error";
        case Type::Void:        return "void";
        case Type::Int:         return "int";
        case Type::Float:       return "float";
        case Type::Bool:        return "bool";
        case Type::SeriesFloat: return "series<float>";
        case Type::SeriesBool:  return "series<bool>";
    }
    return "?";
}

} // namespace aura::lang