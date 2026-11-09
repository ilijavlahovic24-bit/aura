#ifndef AURA_RT_KERNELS_HPP
#define AURA_RT_KERNELS_HPP

#include <cstdint>

namespace aura::rt {

// Elementar operations 
void add_series(const double* a, const double* b, double* out, int64_t n);
void sub_series(const double* a, const double* b, double* out, int64_t n);
void mul_series(const double* a, const double* b, double* out, int64_t n);
void div_series(const double* a, const double* b, double* out, int64_t n);
void neg_series(const double* a, double* out, int64_t n);
void abs_series(const double* a, double* out, int64_t n);

// Indicators
void sma(const double* in, int64_t n, int64_t window, double* out);
void ema(const double* in, int64_t n, int64_t window, double* out);
void stdev(const double* in, int64_t n, int64_t window, double* out);

} // namespace aura::rt

#endif