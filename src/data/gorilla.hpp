#ifndef GORILLA_HPP
#define GORILLA_HPP

#include <cstdint>
#include <cstddef>
#include <vector>

namespace aura::data::aurd::gorilla {

std::vector<uint8_t> compress_ts(const int64_t* ts, size_t n);
void decompress_ts(const uint8_t* data, size_t size, int64_t* out, size_t n);

std::vector<uint8_t> compress_f64(const double* vals, size_t n);
void decompress_f64(const uint8_t* data, size_t size, double* out, size_t n);

} // namespace aura::data::aurd::gorilla

#endif // GORILLA_HPP