#include "rt/kernels.hpp"
#include <cmath>
#include <algorithm>

#ifdef __AVX2__
#include <immintrin.h>
#endif

namespace aura::rt {
// Helper function for horizontal sum of 4 double values
#ifdef __AVX2__
static inline double hsum_pd(__m256d v) {
    __m128d hi = _mm256_extractf128_pd(v, 1);
    __m128d lo = _mm256_castpd256_pd128(v);
    __m128d sum = _mm_add_pd(hi, lo);
    sum = _mm_hadd_pd(sum, sum);
    return _mm_cvtsd_f64(sum);
}
#endif

// ---------------- Elementar Operations ----------------

void add_series(const double* a, const double* b, double* out, int64_t n) {
    int64_t i = 0;
#ifdef __AVX2__
    for (; i <= n - 4; i += 4) {
        __m256d va = _mm256_loadu_pd(a + i);
        __m256d vb = _mm256_loadu_pd(b + i);
        _mm256_storeu_pd(out + i, _mm256_add_pd(va, vb));
    }
#endif
    for (; i < n; ++i) out[i] = a[i] + b[i];
}

void sub_series(const double* a, const double* b, double* out, int64_t n) {
    int64_t i = 0;
#ifdef __AVX2__
    for (; i <= n - 4; i += 4) {
        __m256d va = _mm256_loadu_pd(a + i);
        __m256d vb = _mm256_loadu_pd(b + i);
        _mm256_storeu_pd(out + i, _mm256_sub_pd(va, vb));
    }
#endif
    for (; i < n; ++i) out[i] = a[i] - b[i];
}

void mul_series(const double* a, const double* b, double* out, int64_t n) {
    int64_t i = 0;
#ifdef __AVX2__
    for (; i <= n - 4; i += 4) {
        __m256d va = _mm256_loadu_pd(a + i);
        __m256d vb = _mm256_loadu_pd(b + i);
        _mm256_storeu_pd(out + i, _mm256_mul_pd(va, vb));
    }
#endif
    for (; i < n; ++i) out[i] = a[i] * b[i];
}

void div_series(const double* a, const double* b, double* out, int64_t n) {
    int64_t i = 0;
#ifdef __AVX2__
    for (; i <= n - 4; i += 4) {
        __m256d va = _mm256_loadu_pd(a + i);
        __m256d vb = _mm256_loadu_pd(b + i);
        _mm256_storeu_pd(out + i, _mm256_div_pd(va, vb));
    }
#endif
    for (; i < n; ++i) out[i] = a[i] / b[i];
}

void neg_series(const double* a, double* out, int64_t n) {
    int64_t i = 0;
#ifdef __AVX2__
    __m256d sign = _mm256_set1_pd(-0.0);
    for (; i <= n - 4; i += 4) {
        __m256d va = _mm256_loadu_pd(a + i);
        _mm256_storeu_pd(out + i, _mm256_xor_pd(va, sign));
    }
#endif
    for (; i < n; ++i) out[i] = -a[i];
}

void abs_series(const double* a, double* out, int64_t n) {
    int64_t i = 0;
#ifdef __AVX2__
    __m256d mask = _mm256_set1_pd(-0.0);
    for (; i <= n - 4; i += 4) {
        __m256d va = _mm256_loadu_pd(a + i);
        _mm256_storeu_pd(out + i, _mm256_andnot_pd(mask, va));
    }
#endif
    for (; i < n; ++i) out[i] = std::fabs(a[i]);
}

// ---------------- Indicators ----------------

void sma(const double* in, int64_t n, int64_t window, double* out) {
    if (window <= 0 || window > n) return;
    //First window-1 values are NaN
    for (int64_t i = 0; i < window - 1; ++i) out[i] = std::nan("");
    
    
    double sum = 0.0;
    for (int64_t i = 0; i < window; ++i) sum += in[i];
    out[window - 1] = sum / window;
    
    // Sliding window (scalar, since there is sequential dependency)
    for (int64_t i = window; i < n; ++i) {
        sum += in[i] - in[i - window];
        out[i] = sum / window;
    }
}

void ema(const double* in, int64_t n, int64_t window, double* out) {
    if (window <= 0 || window > n) return;
    for (int64_t i = 0; i < window - 1; ++i) out[i] = std::nan("");
    
    double alpha = 2.0 / (window + 1.0);
    double ema_val = in[window - 1]; // Initialization with the first value
    out[window - 1] = ema_val;
    
    for (int64_t i = window; i < n; ++i) {
        ema_val = alpha * in[i] + (1.0 - alpha) * ema_val;
        out[i] = ema_val;
    }
}

void stdev(const double* in, int64_t n, int64_t window, double* out) {
    if (window <= 0 || window > n) return;
    for (int64_t i = 0; i < window - 1; ++i) out[i] = std::nan("");
    
    double sum = 0.0, sum2 = 0.0;
    for (int64_t i = 0; i < window; ++i) {
        sum += in[i];
        sum2 += in[i] * in[i];
    }
    double mean = sum / window;
    out[window - 1] = std::sqrt(std::max(0.0, (sum2 / window) - (mean * mean)));
    
    for (int64_t i = window; i < n; ++i) {
        double old_val = in[i - window];
        sum += in[i] - old_val;
        sum2 += in[i] * in[i] - old_val * old_val;
        mean = sum / window;
        out[i] = std::sqrt(std::max(0.0, (sum2 / window) - (mean * mean)));
    }
}

} // namespace aura::rt