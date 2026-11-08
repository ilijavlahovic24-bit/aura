#ifndef AURA_CODEGEN_CONTEXT_HPP
#define AURA_CODEGEN_CONTEXT_HPP

#include <cstdint>
#include <cstddef>

struct KernelCtx {
    const double* open;
    const double* high;
    const double* low;
    const double* close;
    const double* volume;
    const double* params;      // A sequence of parameter values
    
    uint8_t* enter_long;
    uint8_t* exit_long;
    uint8_t* enter_short;
    uint8_t* exit_short;
    double*  size;
    
    double*  scratch;          // memory for temporary batches (of size num_scratch * n)
    int64_t  n;                // Number of bars
    int64_t  num_scratch;      
};

typedef void (*KernelFn)(const KernelCtx*);

// Runtime functions called by LLVM
extern "C" {
    // ---- (series × series) ----
    void aura_rt_add(const double* a, const double* b, double* out, int64_t n);
    void aura_rt_sub(const double* a, const double* b, double* out, int64_t n);
    void aura_rt_mul(const double* a, const double* b, double* out, int64_t n);
    void aura_rt_div(const double* a, const double* b, double* out, int64_t n);
    void aura_rt_mod(const double* a, const double* b, double* out, int64_t n);
    void aura_rt_min(const double* a, const double* b, double* out, int64_t n);
    void aura_rt_max(const double* a, const double* b, double* out, int64_t n);

    // ---- Unary (series) ----
    void aura_rt_neg(const double* a, double* out, int64_t n);
    void aura_rt_abs(const double* a, double* out, int64_t n);
    void aura_rt_sqrt(const double* a, double* out, int64_t n);
    void aura_rt_log(const double* a, double* out, int64_t n);

    // ---- Comparassion (series × series → series<bool>) ----
    void aura_rt_lt(const double* a, const double* b, uint8_t* out, int64_t n);
    void aura_rt_le(const double* a, const double* b, uint8_t* out, int64_t n);
    void aura_rt_gt(const double* a, const double* b, uint8_t* out, int64_t n);
    void aura_rt_ge(const double* a, const double* b, uint8_t* out, int64_t n);
    void aura_rt_eq(const double* a, const double* b, uint8_t* out, int64_t n);
    void aura_rt_ne(const double* a, const double* b, uint8_t* out, int64_t n);

    // ---- Boolean (series<bool> × series<bool> → series<bool>) ----
    void aura_rt_and(const uint8_t* a, const uint8_t* b, uint8_t* out, int64_t n);
    void aura_rt_or (const uint8_t* a, const uint8_t* b, uint8_t* out, int64_t n);
    void aura_rt_not(const uint8_t* a, uint8_t* out, int64_t n);

    // ---- Indicators ----
    void aura_rt_sma(const double* in, int64_t n, int64_t window, double* out);
    void aura_rt_ema(const double* in, int64_t n, int64_t window, double* out);
    void aura_rt_stdev(const double* in, int64_t n, int64_t window, double* out);
    void aura_rt_rsi(const double* in, int64_t n, int64_t window, double* out);
    void aura_rt_highest(const double* in, int64_t n, int64_t window, double* out);
    void aura_rt_lowest(const double* in, int64_t n, int64_t window, double* out);
    void aura_rt_shift(const double* in, int64_t n, int64_t k, double* out);
    void aura_rt_crossover(const double* a, const double* b, uint8_t* out, int64_t n);
    void aura_rt_crossunder(const double* a, const double* b, uint8_t* out, int64_t n);
    void aura_rt_atr(const double* high, const double* low, const double* close,
                     int64_t n, int64_t window, double* out);

    // ---- Selection ----
    void aura_rt_where(const uint8_t* cond, const double* a, const double* b,
                       double* out, int64_t n);

    // ---- Broadcast: scalar → series ----
    void aura_rt_broadcast_f64(double v, double* out, int64_t n);
    void aura_rt_broadcast_u8 (uint8_t v, uint8_t* out, int64_t n);

    // ---- Copy: series → izlazni bafer (signali, size) ----
    void aura_rt_copy_u8 (const uint8_t* in, uint8_t* out, int64_t n);
    void aura_rt_copy_f64(const double* in, double* out, int64_t n);
}

#endif