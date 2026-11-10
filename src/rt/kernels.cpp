#include "rt/kernels.hpp"
#include <cmath>
#include <cstring>
#include <algorithm>
#include <vector>

#ifdef __AVX2__
#include <immintrin.h>
#endif

namespace aura::rt {

// ---------- AVX2 element-wise ----------
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

void broadcast_f64(double v, double* out, int64_t n) {
    int64_t i = 0;
#ifdef __AVX2__
    __m256d vv = _mm256_set1_pd(v);
    for (; i <= n - 4; i += 4) _mm256_storeu_pd(out + i, vv);
#endif
    for (; i < n; ++i) out[i] = v;
}

void broadcast_u8(uint8_t v, uint8_t* out, int64_t n) {
    std::memset(out, v, (size_t)n);
}

void copy_u8(const uint8_t* in, uint8_t* out, int64_t n) {
    std::memcpy(out, in, (size_t)n);
}

void copy_f64(const double* in, double* out, int64_t n) {
    std::memcpy(out, in, (size_t)n * sizeof(double));
}

// ---------- Scalar: element-wise aritmetika ----------
void mod_series(const double* a, const double* b, double* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = std::fmod(a[i], b[i]);
}
void min_series(const double* a, const double* b, double* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = std::fmin(a[i], b[i]);
}
void max_series(const double* a, const double* b, double* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = std::fmax(a[i], b[i]);
}
void sqrt_series(const double* a, double* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = std::sqrt(a[i]);
}
void log_series(const double* a, double* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = std::log(a[i]);
}

// ---------- Scalar: poređenja ----------
void lt_series(const double* a, const double* b, uint8_t* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = (a[i] < b[i]) ? 1 : 0;
}
void le_series(const double* a, const double* b, uint8_t* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = (a[i] <= b[i]) ? 1 : 0;
}
void gt_series(const double* a, const double* b, uint8_t* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = (a[i] > b[i]) ? 1 : 0;
}
void ge_series(const double* a, const double* b, uint8_t* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = (a[i] >= b[i]) ? 1 : 0;
}
void eq_series(const double* a, const double* b, uint8_t* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = (a[i] == b[i]) ? 1 : 0;
}
void ne_series(const double* a, const double* b, uint8_t* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = (a[i] != b[i]) ? 1 : 0;
}

// ---------- Scalar: boolean ----------
void and_series(const uint8_t* a, const uint8_t* b, uint8_t* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = (a[i] && b[i]) ? 1 : 0;
}
void or_series(const uint8_t* a, const uint8_t* b, uint8_t* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = (a[i] || b[i]) ? 1 : 0;
}
void not_series(const uint8_t* a, uint8_t* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = a[i] ? 0 : 1;
}

// ---------- Indikatori ----------
void sma(const double* in, int64_t n, int64_t window, double* out) {
    if (window <= 0 || window > n) return;
    for (int64_t i = 0; i < window - 1; ++i) out[i] = std::nan("");
    double sum = 0.0;
    for (int64_t i = 0; i < window; ++i) sum += in[i];
    out[window - 1] = sum / window;
    for (int64_t i = window; i < n; ++i) {
        sum += in[i] - in[i - window];
        out[i] = sum / window;
    }
}

void ema(const double* in, int64_t n, int64_t window, double* out) {
    if (window <= 0 || window > n) return;
    for (int64_t i = 0; i < window - 1; ++i) out[i] = std::nan("");
    double alpha = 2.0 / (window + 1.0);
    double val = in[window - 1];
    out[window - 1] = val;
    for (int64_t i = window; i < n; ++i) {
        val = alpha * in[i] + (1.0 - alpha) * val;
        out[i] = val;
    }
}

void stdev(const double* in, int64_t n, int64_t window, double* out) {
    if (window <= 0 || window > n) return;
    for (int64_t i = 0; i < window - 1; ++i) out[i] = std::nan("");
    double sum = 0.0, sum2 = 0.0;
    for (int64_t i = 0; i < window; ++i) { sum += in[i]; sum2 += in[i] * in[i]; }
    double mean = sum / window;
    out[window - 1] = std::sqrt(std::max(0.0, sum2 / window - mean * mean));
    for (int64_t i = window; i < n; ++i) {
        double old = in[i - window];
        sum  += in[i] - old;
        sum2 += in[i] * in[i] - old * old;
        mean = sum / window;
        out[i] = std::sqrt(std::max(0.0, sum2 / window - mean * mean));
    }
}

void rsi(const double* in, int64_t n, int64_t window, double* out) {
    if (window <= 0 || window + 1 >= n) {
        for (int64_t i = 0; i < n; ++i) out[i] = std::nan("");
        return;
    }
    for (int64_t i = 0; i <= window; ++i) out[i] = std::nan("");
    double gain = 0.0, loss = 0.0;
    for (int64_t i = 1; i <= window; ++i) {
        double d = in[i] - in[i - 1];
        if (d > 0) gain += d; else loss -= d;
    }
    gain /= window; loss /= window;
    out[window] = (loss == 0.0) ? 100.0 : 100.0 - 100.0 / (1.0 + gain / loss);
    for (int64_t i = window + 1; i < n; ++i) {
        double d = in[i] - in[i - 1];
        double g = d > 0 ? d : 0.0;
        double l = d < 0 ? -d : 0.0;
        gain = (gain * (window - 1) + g) / window;
        loss = (loss * (window - 1) + l) / window;
        out[i] = (loss == 0.0) ? 100.0 : 100.0 - 100.0 / (1.0 + gain / loss);
    }
}

void highest(const double* in, int64_t n, int64_t window, double* out) {
    for (int64_t i = 0; i < n; ++i) {
        if (i < window - 1) { out[i] = std::nan(""); continue; }
        double m = in[i];
        for (int64_t j = 1; j < window; ++j) m = std::max(m, in[i - j]);
        out[i] = m;
    }
}

void lowest(const double* in, int64_t n, int64_t window, double* out) {
    for (int64_t i = 0; i < n; ++i) {
        if (i < window - 1) { out[i] = std::nan(""); continue; }
        double m = in[i];
        for (int64_t j = 1; j < window; ++j) m = std::min(m, in[i - j]);
        out[i] = m;
    }
}

void shift(const double* in, int64_t n, int64_t k, double* out) {
    for (int64_t i = 0; i < n; ++i) out[i] = (i >= k) ? in[i - k] : std::nan("");
}

void crossover(const double* a, const double* b, uint8_t* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i)
        out[i] = (i > 0 && a[i-1] <= b[i-1] && a[i] > b[i]) ? 1 : 0;
}

void crossunder(const double* a, const double* b, uint8_t* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i)
        out[i] = (i > 0 && a[i-1] >= b[i-1] && a[i] < b[i]) ? 1 : 0;
}

void atr(const double* h, const double* l, const double* c,
         int64_t n, int64_t window, double* out) {
    if (n == 0) return;
    std::vector<double> tr((size_t)n);
    tr[0] = h[0] - l[0];
    for (int64_t i = 1; i < n; ++i) {
        double a = h[i] - l[i];
        double b = std::fabs(h[i] - c[i-1]);
        double d = std::fabs(l[i] - c[i-1]);
        tr[i] = std::max({a, b, d});
    }
    sma(tr.data(), n, window, out);
}

void where(const uint8_t* cond, const double* a, const double* b,
           double* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = cond[i] ? a[i] : b[i];
}

} // namespace aura::rt