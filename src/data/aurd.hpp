#ifndef AURD_HPP
#define AURD_HPP
#include<cstddef>
#include<cstdint>

namespace aura::data::aurd{
    inline constexpr char kMagic[4]={'A','U','R','D'};
    inline constexpr uint16_t kVersion=1;
    inline constexpr uint16_t  kFlagCompressed=0x001;
    inline constexpr uint32_t kDefaultBlockSize=4096;
    inline constexpr std::size_t  kHeaderSize=64;
    inline constexpr std::size_t kSymbolLen =16;
    inline constexpr std::size_t kAligment =32; // Alignment for AX32
    struct Header{
        char magic[4];//+0
        uint16_t version;//+4
        uint16_t flags;//+6
        uint64_t num_bars;//+8
        uint32_t timeframe_sec;//+16
        uint32_t block_size;//+20
        uint64_t num_blocksl;//+24
        uint64_t index_offset;//+32
        char symbol[16];//+40
        uint64_t reserved;//+56
    };
    static_assert(sizeof(Header) == kHeaderSize, "header must be 64 bytes");
    static_assert(offsetof(Header, index_offset)== 32);
    static_assert(offsetof(Header, symbol)== 40);

    enum class Column: std::uint8_t{Ts=0,Open,High,Low,Close,Volume,Count=6};
    struct BlockEntry{
        uint64_t offset;
        uint32_t size;
        uint32_t num_bars;
    };
    static_assert(sizeof(BlockEntry) == 16);
}




#endif