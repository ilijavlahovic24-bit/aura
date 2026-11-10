#ifndef AURA_RT_KERNELS_HPP
#define AURA_RT_KERNELS_HPP

#include <cstdint>

namespace aura::rt {

// AVX2-optimizovane element-wise operacije
void add_series(const double* a, const double* b, double* out, int64_t n);
void sub_series(const double* a, const double* b, double* out, int64_t n);
void mul_series(const double* a, const double* b, double* out, int64_t n);
void div_series(const double* a, const double* b, double* out, int64_t n);
void neg_series(const double* a, double* out, int64_t n);
void abs_series(const double* a, double* out, int64_t n);
void broadcast_f64(double v, double* out, int64_t n);
void broadcast_u8(uint8_t v, uint8_t* out, int64_t n);
void copy_u8(const uint8_t* in, uint8_t* out, int64_t n);
void copy_f64(const double* in, double* out, int64_t n);

// Scalar (sekvencijalni ili bez AVX2 benefita)
void mod_series(const double* a, const double* b, double* out, int64_t n);
void min_series(const double* a, const double* b, double* out, int64_t n);
void max_series(const double* a, const double* b, double* out, int64_t n);
void sqrt_series(const double* a, double* out, int64_t n);
void log_series(const double* a, double* out, int64_t n);

void lt_series(const double* a, const double* b, uint8_t* out, int64_t n);
void le_series(const double* a, const double* b, uint8_t* out, int64_t n);
void gt_series(const double* a, const double* b, uint8_t* out, int64_t n);
void ge_series(const double* a, const double* b, uint8_t* out, int64_t n);
void eq_series(const double* a, const double* b, uint8_t* out, int64_t n);
void ne_series(const double* a, const double* b, uint8_t* out, int64_t n);

void and_series(const uint8_t* a, const uint8_t* b, uint8_t* out, int64_t n);
void or_series (const uint8_t* a, const uint8_t* b, uint8_t* out, int64_t n);
void not_series(const uint8_t* a, uint8_t* out, int64_t n);

void sma    (const double* in, int64_t n, int64_t w, double* out);
void ema    (const double* in, int64_t n, int64_t w, double* out);
void stdev  (const double* in, int64_t n, int64_t w, double* out);
void rsi    (const double* in, int64_t n, int64_t w, double* out);
void highest(const double* in, int64_t n, int64_t w, double* out);
void lowest (const double* in, int64_t n, int64_t w, double* out);
void shift  (const double* in, int64_t n, int64_t k, double* out);
void crossover (const double* a, const double* b, uint8_t* out, int64_t n);
void crossunder(const double* a, const double* b, uint8_t* out, int64_t n);
void atr(const double* h, const double* l, const double* c,
         int64_t n, int64_t w, double* out);
void where(const uint8_t* cond, const double* a, const double* b,
           double* out, int64_t n);

} // namespace aura::rt

#endif