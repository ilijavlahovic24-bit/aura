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
    if (compress) throw DatasetError("compression not implemented yet");
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
    return d;
}

const double* Dataset::ptr_double(Column c, size_t b, size_t e) const {
    if (b > e || e > num_bars_) throw DatasetError("range out of bounds");
    if (flags_ & kFlagCompressed) throw DatasetError("compressed not implemented");
    if (b == e || num_blocks_ == 0) return nullptr;
    
    const BlockEntry& base = index_[static_cast<size_t>(c)];
    const double* p = reinterpret_cast<const double*>(
        static_cast<const uint8_t*>(mmap_base_) + base.offset);
    
    return p + b;
}

const int64_t* Dataset::ptr_i64(Column c, size_t b, size_t e) const {
    if (b > e || e > num_bars_) throw DatasetError("range out of bounds");
    if (flags_ & kFlagCompressed) throw DatasetError("compressed not implemented");
    if (b == e || num_blocks_ == 0) return nullptr;
    
    const BlockEntry& base = index_[static_cast<size_t>(c)];
    const int64_t* p = reinterpret_cast<const int64_t*>(
        static_cast<const uint8_t*>(mmap_base_) + base.offset);
    
    return p + b;
}

} // namespace aura::data::aurd