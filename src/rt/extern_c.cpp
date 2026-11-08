#include "codegen/context.hpp"
#include <cmath>
#include <cstring>
#include <algorithm>
#include<vector>

extern "C" {

// ---- Aritmetika ----
void aura_rt_add(const double* a, const double* b, double* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = a[i] + b[i];
}
void aura_rt_sub(const double* a, const double* b, double* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = a[i] - b[i];
}
void aura_rt_mul(const double* a, const double* b, double* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = a[i] * b[i];
}
void aura_rt_div(const double* a, const double* b, double* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = a[i] / b[i];
}
void aura_rt_mod(const double* a, const double* b, double* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = std::fmod(a[i], b[i]);
}
void aura_rt_min(const double* a, const double* b, double* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = std::fmin(a[i], b[i]);
}
void aura_rt_max(const double* a, const double* b, double* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = std::fmax(a[i], b[i]);
}

// ---- Unary ----
void aura_rt_neg(const double* a, double* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = -a[i];
}
void aura_rt_abs(const double* a, double* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = std::fabs(a[i]);
}
void aura_rt_sqrt(const double* a, double* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = std::sqrt(a[i]);
}
void aura_rt_log(const double* a, double* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = std::log(a[i]);
}

// ---- Poređenja ----
void aura_rt_lt(const double* a, const double* b, uint8_t* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = (a[i] < b[i]) ? 1 : 0;
}
void aura_rt_le(const double* a, const double* b, uint8_t* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = (a[i] <= b[i]) ? 1 : 0;
}
void aura_rt_gt(const double* a, const double* b, uint8_t* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = (a[i] > b[i]) ? 1 : 0;
}
void aura_rt_ge(const double* a, const double* b, uint8_t* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = (a[i] >= b[i]) ? 1 : 0;
}
void aura_rt_eq(const double* a, const double* b, uint8_t* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = (a[i] == b[i]) ? 1 : 0;
}
void aura_rt_ne(const double* a, const double* b, uint8_t* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = (a[i] != b[i]) ? 1 : 0;
}

// ---- Boolean ----
void aura_rt_and(const uint8_t* a, const uint8_t* b, uint8_t* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = (a[i] && b[i]) ? 1 : 0;
}
void aura_rt_or(const uint8_t* a, const uint8_t* b, uint8_t* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = (a[i] || b[i]) ? 1 : 0;
}
void aura_rt_not(const uint8_t* a, uint8_t* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = a[i] ? 0 : 1;
}

// ---- Indikatori ----
void aura_rt_sma(const double* in, int64_t n, int64_t window, double* out) {
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

void aura_rt_ema(const double* in, int64_t n, int64_t window, double* out) {
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

void aura_rt_stdev(const double* in, int64_t n, int64_t window, double* out) {
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

void aura_rt_rsi(const double* in, int64_t n, int64_t window, double* out) {
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

void aura_rt_highest(const double* in, int64_t n, int64_t window, double* out) {
    for (int64_t i = 0; i < n; ++i) {
        if (i < window - 1) { out[i] = std::nan(""); continue; }
        double m = in[i];
        for (int64_t j = 1; j < window; ++j) m = std::max(m, in[i - j]);
        out[i] = m;
    }
}

void aura_rt_lowest(const double* in, int64_t n, int64_t window, double* out) {
    for (int64_t i = 0; i < n; ++i) {
        if (i < window - 1) { out[i] = std::nan(""); continue; }
        double m = in[i];
        for (int64_t j = 1; j < window; ++j) m = std::min(m, in[i - j]);
        out[i] = m;
    }
}

void aura_rt_shift(const double* in, int64_t n, int64_t k, double* out) {
    for (int64_t i = 0; i < n; ++i) out[i] = (i >= k) ? in[i - k] : std::nan("");
}

void aura_rt_crossover(const double* a, const double* b, uint8_t* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i)
        out[i] = (i > 0 && a[i-1] <= b[i-1] && a[i] > b[i]) ? 1 : 0;
}

void aura_rt_crossunder(const double* a, const double* b, uint8_t* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i)
        out[i] = (i > 0 && a[i-1] >= b[i-1] && a[i] < b[i]) ? 1 : 0;
}

void aura_rt_atr(const double* h, const double* l, const double* c,
                 int64_t n, int64_t window, double* out) {
    if (n == 0) return;
    std::vector<double> tr(n);
    tr[0] = h[0] - l[0];
    for (int64_t i = 1; i < n; ++i) {
        double a = h[i] - l[i];
        double b = std::fabs(h[i] - c[i-1]);
        double d = std::fabs(l[i] - c[i-1]);
        tr[i] = std::max({a, b, d});
    }
    aura_rt_sma(tr.data(), n, window, out);
}

// ---- Selekcija ----
void aura_rt_where(const uint8_t* cond, const double* a, const double* b,
                   double* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = cond[i] ? a[i] : b[i];
}

// ---- Broadcast / copy ----
void aura_rt_broadcast_f64(double v, double* out, int64_t n) {
    for (int64_t i = 0; i < n; ++i) out[i] = v;
}
void aura_rt_broadcast_u8(uint8_t v, uint8_t* out, int64_t n) {
    std::memset(out, v, n);
}
void aura_rt_copy_u8(const uint8_t* in, uint8_t* out, int64_t n) {
    std::memcpy(out, in, n);
}
void aura_rt_copy_f64(const double* in, double* out, int64_t n) {
    std::memcpy(out, in, n * sizeof(double));
}

} // extern "C"