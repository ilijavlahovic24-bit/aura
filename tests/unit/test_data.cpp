#include "data/aurd.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

using namespace aura::data::aurd;

static RawBars make_bars(size_t n) {
    RawBars b;
    b.ts.resize(n); b.open.resize(n); b.high.resize(n);
    b.low.resize(n); b.close.resize(n); b.volume.resize(n);
    for (size_t i = 0; i < n; ++i) {
        b.ts[i]     = 1'700'000'000 + static_cast<int64_t>(i * 60);
        b.open[i]   = 100.0 + static_cast<double>(i);
        b.high[i]   = 101.0 + static_cast<double>(i);
        b.low[i]    =  99.0 + static_cast<double>(i);
        b.close[i]  = 100.5 + static_cast<double>(i);
        b.volume[i] = 1000.0 + static_cast<double>(i);
    }
    b.timeframe_sec = 60;
    b.symbol        = "TEST";
    return b;
}

static RawBars roundtrip(size_t n) {
    auto in = make_bars(n);
    auto tmp = std::filesystem::temp_directory_path() / "aura_test.aurd";
    
    write_dataset(tmp, in, false);
    auto ds = Dataset::open(tmp);
    
    EXPECT_EQ(ds.size(), n);
    
    RawBars out;
    if (n > 0) {
        const int64_t* ts_ptr = ds.ts(0, n);
        out.ts.assign(ts_ptr, ts_ptr + n);
        
        const double* close_ptr = ds.close(0, n);
        out.close.assign(close_ptr, close_ptr + n);
    }
    
    std::filesystem::remove(tmp);
    return out;
}

TEST(Aurd, HeaderStable) {
    EXPECT_EQ(sizeof(Header), 64u);
    EXPECT_EQ(sizeof(BlockEntry), 16u);
}

TEST(Aurd, RoundTrip1Bar) {
    auto out = roundtrip(1);
    EXPECT_EQ(out.ts[0], 1'700'000'000);
    EXPECT_DOUBLE_EQ(out.close[0], 100.5);
}

TEST(Aurd, RoundTripBlockBoundary) {
    auto out = roundtrip(4096); // Tačno jedan blok
    EXPECT_DOUBLE_EQ(out.close[4095], 100.5 + 4095.0);
}

TEST(Aurd, RoundTripAcrossBlocks) {
    auto out = roundtrip(4097); // Blok + 1 bar
    EXPECT_DOUBLE_EQ(out.close[4096], 100.5 + 4096.0);
}

TEST(Aurd, ZeroCopyPointer) {
    auto in = make_bars(5000);
    auto tmp = std::filesystem::temp_directory_path() / "aura_test.aurd";
    
    write_dataset(tmp, in, false);
    auto ds = Dataset::open(tmp);
    
    // Uzimamo opseg od 100. do 4900. bara (dužina 4800).
    // Pošto je kolona fizički kontinualna, ovo mora biti samo pokazivač + offset.
    const double* s = ds.close(100, 4900);
    ASSERT_NE(s, nullptr);
    
    EXPECT_DOUBLE_EQ(s[0],      100.5 + 100.0);  // Prvi element opsega
    EXPECT_DOUBLE_EQ(s[4799],   100.5 + 4899.0); // Poslednji (4900 - 100 - 1)
    
    std::filesystem::remove(tmp);
}

TEST(Aurd, CorruptMagic) {
    auto in = make_bars(10);
    auto tmp = std::filesystem::temp_directory_path() / "aura_bad.aurd";
    write_dataset(tmp, in, false);
    
    // Namerno kvarimo prvi bajt (magic 'A' -> 'X')
    {
        std::fstream f(tmp, std::ios::in | std::ios::out | std::ios::binary);
        f.seekp(0); 
        f.put('X');
    }
    
    // Očekujemo da open baci DatasetError, a ne da padne program (SIGSEGV)
    EXPECT_THROW(Dataset::open(tmp), DatasetError);
    
    std::filesystem::remove(tmp);
}