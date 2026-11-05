#include "data/gorilla.hpp"
#include <cstring>
#include <stdexcept>

namespace aura::data::aurd::gorilla {

class BitWriter {
    std::vector<uint8_t> buf_;
    size_t bit_pos_ = 0;
public:
    void write_bit(uint32_t bit) {
        if (bit_pos_ % 8 == 0) buf_.push_back(0);
        if (bit) buf_.back() |= (1 << (7 - (bit_pos_ % 8)));
        ++bit_pos_;
    }
    void write_bits(uint64_t value, int n) {
        for (int i = n - 1; i >= 0; --i) write_bit((value >> i) & 1);
    }
    std::vector<uint8_t> finish() { return std::move(buf_); }
};

class BitReader {
    const uint8_t* data_;
    size_t size_;
    size_t bit_pos_ = 0;
public:
    BitReader(const uint8_t* data, size_t size) : data_(data), size_(size) {}
    uint64_t read_bit() {
        if (bit_pos_ >= size_ * 8) throw std::runtime_error("bitstream eof");
        uint32_t bit = (data_[bit_pos_ / 8] >> (7 - (bit_pos_ % 8))) & 1;
        ++bit_pos_;
        return bit;
    }
    uint64_t read_bits(int n) {
        uint64_t v = 0;
        for (int i = 0; i < n; ++i) v = (v << 1) | read_bit();
        return v;
    }
};

static int64_t sign_extend(uint64_t val, int bits) {
    uint64_t mask = 1ULL << (bits - 1);
    return static_cast<int64_t>((val ^ mask) - mask);
}

std::vector<uint8_t> compress_ts(const int64_t* ts, size_t n) {
    if (n == 0) return {};
    BitWriter bw;
    if (n == 1) { bw.write_bits(ts[0], 64); return bw.finish(); }
    bw.write_bits(ts[0], 64);
    int64_t d1 = ts[1] - ts[0];
    bw.write_bits(static_cast<uint64_t>(d1), 64);
    int64_t prev_d = d1;
    for (size_t i = 2; i < n; ++i) {
        int64_t d = ts[i] - ts[i-1];
        int64_t dod = d - prev_d;
        if (dod == 0) {
            bw.write_bit(0);
        } else if (dod >= -64 && dod <= 63) {
            bw.write_bits(0b10, 2);
            bw.write_bits(static_cast<uint64_t>(dod) & 0x7F, 7);
        } else if (dod >= -256 && dod <= 255) {
            bw.write_bits(0b110, 3);
            bw.write_bits(static_cast<uint64_t>(dod) & 0x1FF, 9);
        } else if (dod >= -2048 && dod <= 2047) {
            bw.write_bits(0b1110, 4);
            bw.write_bits(static_cast<uint64_t>(dod) & 0xFFF, 12);
        } else {
            bw.write_bits(0b1111, 4);
            bw.write_bits(static_cast<uint64_t>(dod), 64);
        }
        prev_d = d;
    }
    return bw.finish();
}

void decompress_ts(const uint8_t* data, size_t size, int64_t* out, size_t n) {
    if (n == 0) return;
    BitReader br(data, size);
    out[0] = static_cast<int64_t>(br.read_bits(64));
    if (n == 1) return;
    int64_t d1 = static_cast<int64_t>(br.read_bits(64));
    out[1] = out[0] + d1;
    int64_t prev_d = d1;
    for (size_t i = 2; i < n; ++i) {
        uint64_t b = br.read_bit();
        int64_t dod = 0;
        if (b == 0) {
            dod = 0;
        } else {
            uint64_t b2 = br.read_bit();
            if (b2 == 0) {
                dod = sign_extend(br.read_bits(7), 7);
            } else {
                uint64_t b3 = br.read_bit();
                if (b3 == 0) {
                    dod = sign_extend(br.read_bits(9), 9);
                } else {
                    uint64_t b4 = br.read_bit();
                    if (b4 == 0) {
                        dod = sign_extend(br.read_bits(12), 12);
                    } else {
                        dod = static_cast<int64_t>(br.read_bits(64));
                    }
                }
            }
        }
        int64_t d = prev_d + dod;
        out[i] = out[i-1] + d;
        prev_d = d;
    }
}

std::vector<uint8_t> compress_f64(const double* vals, size_t n) {
    if (n == 0) return {};
    BitWriter bw;
    uint64_t prev_val;
    std::memcpy(&prev_val, &vals[0], 8);
    bw.write_bits(prev_val, 64);
    if (n == 1) return bw.finish();
    int prev_lz = -1, prev_tz = -1;
    for (size_t i = 1; i < n; ++i) {
        uint64_t val;
        std::memcpy(&val, &vals[i], 8);
        uint64_t xor_val = val ^ prev_val;
        if (xor_val == 0) {
            bw.write_bit(0);
        } else {
            bw.write_bit(1);
            // C++17: koristimo ugrađene funkcije kompajlera
            int lz = __builtin_clzll(xor_val);
            int tz = __builtin_ctzll(xor_val);
            if (prev_lz != -1 && lz >= prev_lz && tz >= prev_tz) {
                bw.write_bit(0);
                int meaningful = 64 - prev_lz - prev_tz;
                bw.write_bits(xor_val >> prev_tz, meaningful);
            } else {
                bw.write_bit(1);
                int meaningful = 64 - lz - tz;
                bw.write_bits(lz, 6);
                bw.write_bits(meaningful - 1, 6);
                bw.write_bits(xor_val >> tz, meaningful);
                prev_lz = lz;
                prev_tz = tz;
            }
        }
        prev_val = val;
    }
    return bw.finish();
}

void decompress_f64(const uint8_t* data, size_t size, double* out, size_t n) {
    if (n == 0) return;
    BitReader br(data, size);
    uint64_t prev_val = br.read_bits(64);
    std::memcpy(&out[0], &prev_val, 8);
    if (n == 1) return;
    int prev_lz = -1, prev_tz = -1;
    for (size_t i = 1; i < n; ++i) {
        uint64_t b = br.read_bit();
        uint64_t val;
        if (b == 0) {
            val = prev_val;
        } else {
            uint64_t b2 = br.read_bit();
            uint64_t xor_val;
            if (b2 == 0) {
                int meaningful = 64 - prev_lz - prev_tz;
                xor_val = br.read_bits(meaningful) << prev_tz;
            } else {
                int lz = br.read_bits(6);
                int meaningful = br.read_bits(6) + 1;
                int tz = 64 - lz - meaningful;
                xor_val = br.read_bits(meaningful) << tz;
                prev_lz = lz;
                prev_tz = tz;
            }
            val = prev_val ^ xor_val;
        }
        std::memcpy(&out[i], &val, 8);
        prev_val = val;
    }
}

} // namespace aura::data::aurd::gorilla