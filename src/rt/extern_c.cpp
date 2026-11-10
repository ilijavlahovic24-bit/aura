#include "codegen/context.hpp"
#include "rt/kernels.hpp"

extern "C" {

void aura_rt_add(const double* a, const double* b, double* out, int64_t n)  { aura::rt::add_series(a, b, out, n); }
void aura_rt_sub(const double* a, const double* b, double* out, int64_t n)  { aura::rt::sub_series(a, b, out, n); }
void aura_rt_mul(const double* a, const double* b, double* out, int64_t n)  { aura::rt::mul_series(a, b, out, n); }
void aura_rt_div(const double* a, const double* b, double* out, int64_t n)  { aura::rt::div_series(a, b, out, n); }
void aura_rt_mod(const double* a, const double* b, double* out, int64_t n)  { aura::rt::mod_series(a, b, out, n); }
void aura_rt_min(const double* a, const double* b, double* out, int64_t n)  { aura::rt::min_series(a, b, out, n); }
void aura_rt_max(const double* a, const double* b, double* out, int64_t n)  { aura::rt::max_series(a, b, out, n); }

void aura_rt_neg(const double* a, double* out, int64_t n)  { aura::rt::neg_series(a, out, n); }
void aura_rt_abs(const double* a, double* out, int64_t n)  { aura::rt::abs_series(a, out, n); }
void aura_rt_sqrt(const double* a, double* out, int64_t n) { aura::rt::sqrt_series(a, out, n); }
void aura_rt_log(const double* a, double* out, int64_t n)  { aura::rt::log_series(a, out, n); }

void aura_rt_lt(const double* a, const double* b, uint8_t* out, int64_t n)  { aura::rt::lt_series(a, b, out, n); }
void aura_rt_le(const double* a, const double* b, uint8_t* out, int64_t n)  { aura::rt::le_series(a, b, out, n); }
void aura_rt_gt(const double* a, const double* b, uint8_t* out, int64_t n)  { aura::rt::gt_series(a, b, out, n); }
void aura_rt_ge(const double* a, const double* b, uint8_t* out, int64_t n)  { aura::rt::ge_series(a, b, out, n); }
void aura_rt_eq(const double* a, const double* b, uint8_t* out, int64_t n)  { aura::rt::eq_series(a, b, out, n); }
void aura_rt_ne(const double* a, const double* b, uint8_t* out, int64_t n)  { aura::rt::ne_series(a, b, out, n); }

void aura_rt_and(const uint8_t* a, const uint8_t* b, uint8_t* out, int64_t n) { aura::rt::and_series(a, b, out, n); }
void aura_rt_or (const uint8_t* a, const uint8_t* b, uint8_t* out, int64_t n) { aura::rt::or_series(a, b, out, n); }
void aura_rt_not(const uint8_t* a, uint8_t* out, int64_t n)                    { aura::rt::not_series(a, out, n); }

void aura_rt_sma(const double* in, int64_t n, int64_t w, double* out)     { aura::rt::sma(in, n, w, out); }
void aura_rt_ema(const double* in, int64_t n, int64_t w, double* out)     { aura::rt::ema(in, n, w, out); }
void aura_rt_stdev(const double* in, int64_t n, int64_t w, double* out)   { aura::rt::stdev(in, n, w, out); }
void aura_rt_rsi(const double* in, int64_t n, int64_t w, double* out)     { aura::rt::rsi(in, n, w, out); }
void aura_rt_highest(const double* in, int64_t n, int64_t w, double* out) { aura::rt::highest(in, n, w, out); }
void aura_rt_lowest(const double* in, int64_t n, int64_t w, double* out)  { aura::rt::lowest(in, n, w, out); }
void aura_rt_shift(const double* in, int64_t n, int64_t k, double* out)   { aura::rt::shift(in, n, k, out); }
void aura_rt_crossover(const double* a, const double* b, uint8_t* out, int64_t n)  { aura::rt::crossover(a, b, out, n); }
void aura_rt_crossunder(const double* a, const double* b, uint8_t* out, int64_t n) { aura::rt::crossunder(a, b, out, n); }
void aura_rt_atr(const double* h, const double* l, const double* c, int64_t n, int64_t w, double* out) {
    aura::rt::atr(h, l, c, n, w, out);
}

void aura_rt_where(const uint8_t* cond, const double* a, const double* b, double* out, int64_t n) {
    aura::rt::where(cond, a, b, out, n);
}

void aura_rt_broadcast_f64(double v, double* out, int64_t n)     { aura::rt::broadcast_f64(v, out, n); }
void aura_rt_broadcast_u8 (uint8_t v, uint8_t* out, int64_t n)   { aura::rt::broadcast_u8(v, out, n); }
void aura_rt_copy_u8 (const uint8_t* in, uint8_t* out, int64_t n) { aura::rt::copy_u8(in, out, n); }
void aura_rt_copy_f64(const double* in, double* out, int64_t n)   { aura::rt::copy_f64(in, out, n); }

} // extern "C"