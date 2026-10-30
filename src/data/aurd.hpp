#ifndef AURD_HPP
#define AURD_HPP

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace aura::data::aurd {

    inline constexpr char kMagic[4] = {'A','U','R','D'};
    inline constexpr uint16_t kVersion = 1;
    inline constexpr uint16_t kFlagCompressed = 0x0001;
    inline constexpr uint32_t kDefaultBlockSize = 4096;
    inline constexpr std::size_t kHeaderSize = 64;
    inline constexpr std::size_t kSymbolLen = 16;
    inline constexpr std::size_t kAlignment = 32; // AVX2 alignment

    struct Header {
        char magic[4];          // +0
        uint16_t version;       // +4
        uint16_t flags;         // +6
        uint64_t num_bars;      // +8
        uint32_t timeframe_sec; // +16
        uint32_t block_size;    // +20
        uint64_t num_blocks;    // +24
        uint64_t index_offset;  // +32
        char symbol[16];        // +40
        uint64_t reserved;      // +56
    };
    static_assert(sizeof(Header) == kHeaderSize, "header must be 64 bytes");
    static_assert(offsetof(Header, index_offset) == 32);
    static_assert(offsetof(Header, symbol) == 40);

    enum class Column : std::uint8_t { Ts = 0, Open, High, Low, Close, Volume, Count = 6 };

    struct BlockEntry {
        uint64_t offset;
        uint32_t size;
        uint32_t num_bars;
    };
    static_assert(sizeof(BlockEntry) == 16);

    struct DatasetError : std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    struct RawBars {
        std::vector<int64_t> ts;
        std::vector<double>  open, high, low, close, volume;
        uint32_t             timeframe_sec = 0;
        std::string          symbol;
    };

    void write_dataset(const std::filesystem::path& out,
                       const RawBars& bars,
                       bool compress);

    void import_csv(const std::filesystem::path& csv,
                    const std::filesystem::path& out,
                    bool compress);

    class Dataset {
    public:
        Dataset() = default;
        ~Dataset();

        Dataset(Dataset&&) noexcept;
        Dataset& operator=(Dataset&&) noexcept;
        Dataset(const Dataset&) = delete;
        Dataset& operator=(const Dataset&) = delete;

        static Dataset open(const std::filesystem::path& path);

        size_t           size()      const { return num_bars_; }
        uint32_t         timeframe() const { return timeframe_sec_; }
        std::string_view symbol()    const { return symbol_; }

        // Vraćaju pokazivač na početak traženog opsega.
        // Pozivalac je odgovoran za poznavanje dužine (end - begin).
        const double*  close (size_t begin, size_t end) const { return ptr_double(Column::Close,  begin, end); }
        const double*  open  (size_t begin, size_t end) const { return ptr_double(Column::Open,   begin, end); }
        const double*  high  (size_t begin, size_t end) const { return ptr_double(Column::High,   begin, end); }
        const double*  low   (size_t begin, size_t end) const { return ptr_double(Column::Low,    begin, end); }
        const double*  volume(size_t begin, size_t end) const { return ptr_double(Column::Volume, begin, end); }
        const int64_t* ts    (size_t begin, size_t end) const { return ptr_i64   (Column::Ts,     begin, end); }

    private:
        const double*  ptr_double(Column c, size_t b, size_t e) const;
        const int64_t* ptr_i64   (Column c, size_t b, size_t e) const;

        void*    mmap_base_ = nullptr;
        size_t   mmap_size_ = 0;
        int      fd_        = -1;
        size_t   num_bars_  = 0;
        uint32_t timeframe_sec_ = 0;
        uint32_t block_size_    = 0;
        uint64_t num_blocks_    = 0;
        uint16_t flags_         = 0;
        std::string symbol_;
        const BlockEntry* index_ = nullptr;
    };

} // namespace aura::data::aurd

#endif // AURD_HPP