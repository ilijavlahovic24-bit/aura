#include "data/aurd.hpp"

#include <algorithm>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace aura::data::aurd {

// ---------------- Writer ----------------
void write_dataset(const std::filesystem::path& out,
                   const RawBars& bars,
                   bool compress)
{
     if (compress) {
        const uint64_t n = bars.ts.size();
        const uint64_t num_blocks = (n + kDefaultBlockSize - 1) / kDefaultBlockSize;

        std::ofstream f(out, std::ios::binary);
        if (!f) throw DatasetError("cannot open for writing");

        Header h{};
        std::memcpy(h.magic, kMagic, 4);
        h.version = kVersion;
        h.flags = kFlagCompressed;
        h.num_bars = n;
        h.timeframe_sec = bars.timeframe_sec;
        h.block_size = kDefaultBlockSize;
        h.num_blocks = num_blocks;
        h.reserved = 0;
        std::memcpy(h.symbol, bars.symbol.c_str(), std::min<size_t>(bars.symbol.size(), kSymbolLen - 1));
        f.write(reinterpret_cast<const char*>(&h), sizeof(Header));

        std::vector<BlockEntry> index;
        index.reserve(num_blocks * 6);

        for (uint64_t b = 0; b < num_blocks; ++b) {
            size_t start = b * kDefaultBlockSize;
            size_t count = std::min<uint64_t>(kDefaultBlockSize, n - start);
            
            for (int c = 0; c < 6; ++c) {
                std::vector<uint8_t> comp;
                if (c == 0) comp = gorilla::compress_ts(bars.ts.data() + start, count);
                else if (c == 1) comp = gorilla::compress_f64(bars.open.data() + start, count);
                else if (c == 2) comp = gorilla::compress_f64(bars.high.data() + start, count);
                else if (c == 3) comp = gorilla::compress_f64(bars.low.data() + start, count);
                else if (c == 4) comp = gorilla::compress_f64(bars.close.data() + start, count);
                else if (c == 5) comp = gorilla::compress_f64(bars.volume.data() + start, count);

                BlockEntry e{};
                e.offset = static_cast<uint64_t>(f.tellp());
                e.size = static_cast<uint32_t>(comp.size());
                e.num_bars = static_cast<uint32_t>(count);
                f.write(reinterpret_cast<const char*>(comp.data()), comp.size());
                index.push_back(e);
            }
        }
        
        h.index_offset = static_cast<uint64_t>(f.tellp());
        f.write(reinterpret_cast<const char*>(index.data()), index.size() * sizeof(BlockEntry));
        
        f.seekp(0);
        f.write(reinterpret_cast<const char*>(&h), sizeof(Header));
        return;
    }
    if (bars.ts.size() != bars.open.size() || bars.ts.size() != bars.close.size())
        throw DatasetError("column size mismatch");

    const uint64_t n = bars.ts.size();
    const uint32_t block_size = kDefaultBlockSize;
    const uint64_t num_blocks = (n + block_size - 1) / block_size;

    std::ofstream f(out, std::ios::binary);
    if (!f) throw DatasetError("cannot open for writing: " + out.string());

    Header h{};
    std::memcpy(h.magic, kMagic, 4);
    h.version        = kVersion;
    h.flags          = 0;
    h.num_bars       = n;
    h.timeframe_sec  = bars.timeframe_sec;
    h.block_size     = block_size;
    h.num_blocks     = num_blocks;
    h.index_offset   = 0;
    h.reserved       = 0;
    std::memcpy(h.symbol, bars.symbol.c_str(),
                std::min<size_t>(bars.symbol.size(), kSymbolLen - 1));
    f.write(reinterpret_cast<const char*>(&h), sizeof(Header));

    auto write_aligned = [&](const void* p, size_t bytes) -> uint64_t {
        const auto pos = static_cast<uint64_t>(f.tellp());
        const size_t pad = (kAlignment - (pos % kAlignment)) % kAlignment;
        static const char zeros[kAlignment] = {0};
        f.write(zeros, static_cast<std::streamsize>(pad));
        const uint64_t off = static_cast<uint64_t>(f.tellp());
        f.write(static_cast<const char*>(p), static_cast<std::streamsize>(bytes));
        return off;
    };

    uint64_t off[6];
    off[0] = write_aligned(bars.ts.data(),     n * sizeof(int64_t));
    off[1] = write_aligned(bars.open.data(),   n * sizeof(double));
    off[2] = write_aligned(bars.high.data(),   n * sizeof(double));
    off[3] = write_aligned(bars.low.data(),    n * sizeof(double));
    off[4] = write_aligned(bars.close.data(),  n * sizeof(double));
    off[5] = write_aligned(bars.volume.data(), n * sizeof(double));

    // Align index to 8 bytes
    {
        const auto pos = static_cast<uint64_t>(f.tellp());
        const size_t pad = (8 - (pos % 8)) % 8;
        static const char zeros[8] = {0};
        f.write(zeros, static_cast<std::streamsize>(pad));
    }
    h.index_offset = static_cast<uint64_t>(f.tellp());

    for (uint64_t b = 0; b < num_blocks; ++b) {
        const uint64_t start = b * block_size;
        const uint32_t bars_in_block =
            static_cast<uint32_t>(std::min<uint64_t>(block_size, n - start));
        for (int c = 0; c < 6; ++c) {
            BlockEntry e{};
            e.offset   = off[c] + start * 8;
            e.size     = bars_in_block * 8;
            e.num_bars = bars_in_block;
            f.write(reinterpret_cast<const char*>(&e), sizeof(BlockEntry));
        }
    }

    f.seekp(0);
    f.write(reinterpret_cast<const char*>(&h), sizeof(Header));
    if (!f) throw DatasetError("write failed");
}

// ---------------- Dataset ----------------
Dataset::~Dataset() {
    if (mmap_base_) ::munmap(mmap_base_, mmap_size_);
    if (fd_ >= 0)   ::close(fd_);
}

Dataset::Dataset(Dataset&& o) noexcept { *this = std::move(o); }

Dataset& Dataset::operator=(Dataset&& o) noexcept {
    if (this != &o) {
        if (mmap_base_) ::munmap(mmap_base_, mmap_size_);
        if (fd_ >= 0)   ::close(fd_);
        mmap_base_     = o.mmap_base_;     o.mmap_base_ = nullptr;
        mmap_size_     = o.mmap_size_;     o.mmap_size_ = 0;
        fd_            = o.fd_;            o.fd_        = -1;
        num_bars_      = o.num_bars_;
        timeframe_sec_ = o.timeframe_sec_;
        block_size_    = o.block_size_;
        num_blocks_    = o.num_blocks_;
        flags_         = o.flags_;
        symbol_        = std::move(o.symbol_);
        index_         = o.index_;         o.index_     = nullptr;
    }
    return *this;
}

Dataset Dataset::open(const std::filesystem::path& path) {
    const int fd = ::open(path.c_str(), O_RDONLY);
    if (fd < 0) throw DatasetError("open failed: " + path.string());

    struct stat st{};
    if (::fstat(fd, &st) != 0) { ::close(fd); throw DatasetError("fstat failed"); }
    const size_t fsize = static_cast<size_t>(st.st_size);
    if (fsize < sizeof(Header)) { ::close(fd); throw DatasetError("file too small"); }

    void* addr = ::mmap(nullptr, fsize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED) { ::close(fd); throw DatasetError("mmap failed"); }

    const auto* h = static_cast<const Header*>(addr);
    if (std::memcmp(h->magic, kMagic, 4) != 0) { ::munmap(addr, fsize); ::close(fd); throw DatasetError("bad magic"); }
    if (h->version != kVersion)                { ::munmap(addr, fsize); ::close(fd); throw DatasetError("bad version"); }
    if (h->reserved != 0)                      { ::munmap(addr, fsize); ::close(fd); throw DatasetError("reserved not zero"); }
    if (h->block_size == 0)                    { ::munmap(addr, fsize); ::close(fd); throw DatasetError("zero block size"); }
    
    const uint64_t idx_bytes = h->num_blocks * 6 * sizeof(BlockEntry);
    if (h->index_offset > fsize || idx_bytes > fsize - h->index_offset) {
        ::munmap(addr, fsize); ::close(fd);
        throw DatasetError("index out of bounds");
    }

    Dataset d;
    d.fd_            = fd;
    d.mmap_base_     = addr;
    d.mmap_size_     = fsize;
    d.num_bars_      = h->num_bars;
    d.timeframe_sec_ = h->timeframe_sec;
    d.block_size_    = h->block_size;
    d.num_blocks_    = h->num_blocks;
    d.flags_         = h->flags;
    d.symbol_        = std::string(h->symbol, ::strnlen(h->symbol, kSymbolLen));
    d.index_         = reinterpret_cast<const BlockEntry*>(
        static_cast<const uint8_t*>(addr) + h->index_offset);
    
     if (d.flags_ & kFlagCompressed) {
        d.decomp_ts_.resize(d.num_bars_);
        d.decomp_open_.resize(d.num_bars_);
        d.decomp_high_.resize(d.num_bars_);
        d.decomp_low_.resize(d.num_bars_);
        d.decomp_close_.resize(d.num_bars_);
        d.decomp_volume_.resize(d.num_bars_);

        for (uint64_t b = 0; b < d.num_blocks_; ++b) {
            size_t start = b * d.block_size_;
            for (int c = 0; c < 6; ++c) {
                const BlockEntry& e = d.index_[b * 6 + c];
                const uint8_t* ptr = static_cast<const uint8_t*>(addr) + e.offset;
                if (c == 0) gorilla::decompress_ts(ptr, e.size, d.decomp_ts_.data() + start, e.num_bars);
                else if (c == 1) gorilla::decompress_f64(ptr, e.size, d.decomp_open_.data() + start, e.num_bars);
                else if (c == 2) gorilla::decompress_f64(ptr, e.size, d.decomp_high_.data() + start, e.num_bars);
                else if (c == 3) gorilla::decompress_f64(ptr, e.size, d.decomp_low_.data() + start, e.num_bars);
                else if (c == 4) gorilla::decompress_f64(ptr, e.size, d.decomp_close_.data() + start, e.num_bars);
                else if (c == 5) gorilla::decompress_f64(ptr, e.size, d.decomp_volume_.data() + start, e.num_bars);
            }
        }
    }
    return d;
}

const double* Dataset::ptr_double(Column c, size_t b, size_t e) const {
    

    if (b > e || e > num_bars_) throw DatasetError("range out of bounds");
    if (b == e || num_blocks_ == 0) return nullptr;
    
    if (flags_ & kFlagCompressed) {
        if (c == Column::Open) return decomp_open_.data() + b;
        if (c == Column::High) return decomp_high_.data() + b;
        if (c == Column::Low) return decomp_low_.data() + b;
        if (c == Column::Close) return decomp_close_.data() + b;
        if (c == Column::Volume) return decomp_volume_.data() + b;
        throw DatasetError("invalid column");
    }

    
    const BlockEntry& base = index_[static_cast<size_t>(c)];
    const double* p = reinterpret_cast<const double*>(
        static_cast<const uint8_t*>(mmap_base_) + base.offset);
    
    return p + b;
}

const int64_t* Dataset::ptr_i64(Column c, size_t b, size_t e) const {
    if (b > e || e > num_bars_) throw DatasetError("range out of bounds");
    if (b == e || num_blocks_ == 0) return nullptr;

    if (c != Column::Ts)
        throw DatasetError("ptr_i64 supports only Column::Ts");

    if (flags_ & kFlagCompressed)
        return decomp_ts_.data() + b;

    const BlockEntry& base = index_[static_cast<size_t>(c)];
    const int64_t* p = reinterpret_cast<const int64_t*>(
        static_cast<const uint8_t*>(mmap_base_) + base.offset);
    return p + b;
}

} // namespace aura::data::aurd