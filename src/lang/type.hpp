#ifndef AURA_LANG_TYPE_HPP
#define AURA_LANG_TYPE_HPP

#include <cstdint>
#include <string>

namespace aura::lang {

enum class Type : uint8_t {
    Error = 0,
    Void,
    Int,
    Float,
    Bool,
    SeriesFloat,
    SeriesBool
};

inline bool is_scalar(Type t) { return t == Type::Int || t == Type::Float || t == Type::Bool; }
inline bool is_series(Type t) { return t == Type::SeriesFloat || t == Type::SeriesBool; }
inline bool is_numeric(Type t) { return t == Type::Int || t == Type::Float || t == Type::SeriesFloat; }
inline bool is_bool(Type t) { return t == Type::Bool || t == Type::SeriesBool; }
inline Type to_series(Type t) {
    if (t == Type::Int || t == Type::Float) return Type::SeriesFloat;
    if (t == Type::Bool) return Type::SeriesBool;
    return t;
}

const char* type_name(Type t);

} // namespace aura::lang

#endif