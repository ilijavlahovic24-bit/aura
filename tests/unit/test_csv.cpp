#include "data/aurd.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

using namespace aura::data::aurd;

static std::filesystem::path write_csv(const std::string& content, const std::string& name) {
    auto p = std::filesystem::temp_directory_path() / name;
    std::ofstream f(p, std::ios::binary);
    f << content;
    return p;
}

struct CsvFixture : ::testing::Test {
    std::vector<std::filesystem::path> cleanup;
    std::filesystem::path csv(const std::string& c, const std::string& name) {
        auto p = write_csv(c, name);
        cleanup.push_back(p);
        return p;
    }
    std::filesystem::path aur(const std::string& name) {
        auto p = std::filesystem::temp_directory_path() / name;
        cleanup.push_back(p);
        return p;
    }
    void TearDown() override {
        for (auto& p : cleanup) { std::error_code ec; std::filesystem::remove(p, ec); }
    }
};

TEST_F(CsvFixture, BasicRoundtrip) {
    auto c = csv("1700000000,100,101,99,100.5,1000\n"
                 "1700000060,101,102,100,101.5,1100\n",
                 "basic.csv");
    auto a = aur("basic.aurd");
    import_csv(c, a, false);
    auto ds = Dataset::open(a);
    EXPECT_EQ(ds.size(), 2u);
    EXPECT_EQ(ds.timeframe(), 60u);
    EXPECT_DOUBLE_EQ(ds.close(0, 2)[0], 100.5);
    EXPECT_DOUBLE_EQ(ds.close(0, 2)[1], 101.5);
}

TEST_F(CsvFixture, IsoTimestamp) {
    auto c = csv("2023-11-14T22:13:20Z,100,101,99,100.5,1000\n"
                 "2023-11-14T22:14:20Z,101,102,100,101.5,1100\n",
                 "iso.csv");
    auto a = aur("iso.aurd");
    import_csv(c, a, false);
    auto ds = Dataset::open(a);
    EXPECT_EQ(ds.ts(0, 1)[0], 1700000000);
}

TEST_F(CsvFixture, HeaderSkipped) {
    auto c = csv("timestamp,open,high,low,close,volume\n"
                 "1700000000,100,101,99,100.5,1000\n",
                 "hdr.csv");
    auto a = aur("hdr.aurd");
    import_csv(c, a, false);
    auto ds = Dataset::open(a);
    EXPECT_EQ(ds.size(), 1u);
}

TEST_F(CsvFixture, ErrorNotIncreasing) {
    auto c = csv("1700000000,100,101,99,100.5,1000\n"
                 "1700000000,101,102,100,101.5,1100\n",
                 "ninc.csv");
    try {
        import_csv(c, aur("ninc.aurd"), false);
        FAIL() << "expected throw";
    } catch (const DatasetError& e) {
        EXPECT_NE(std::string(e.what()).find(":2:"), std::string::npos);
    }
}

TEST_F(CsvFixture, ErrorBadLow) {
    auto c = csv("1700000000,100,101,105,100.5,1000\n", "low.csv");
    EXPECT_THROW(import_csv(c, aur("low.aurd"), false), DatasetError);
}

TEST_F(CsvFixture, ErrorBadHigh) {
    auto c = csv("1700000000,100,99,98,100.5,1000\n", "high.csv");
    EXPECT_THROW(import_csv(c, aur("high.aurd"), false), DatasetError);
}

TEST_F(CsvFixture, ErrorWrongFieldCount) {
    auto c = csv("1700000000,100,101,99,100.5\n", "few.csv");
    EXPECT_THROW(import_csv(c, aur("few.aurd"), false), DatasetError);
}

TEST_F(CsvFixture, ErrorLineNumberInHeaderFile) {
    // Header on line 1, valid line on 2, error on 3
    auto c = csv("timestamp,open,high,low,close,volume\n"
                 "1700000000,100,101,99,100.5,1000\n"
                 "1699999999,101,102,100,101.5,1100\n",
                 "mixed.csv");
    try {
        import_csv(c, aur("mixed.aurd"), false);
        FAIL() << "expected throw";
    } catch (const DatasetError& e) {
        EXPECT_NE(std::string(e.what()).find(":3:"), std::string::npos);
    }
}