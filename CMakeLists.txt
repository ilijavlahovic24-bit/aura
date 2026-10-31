cmake_minimum_required(VERSION 3.22)
project(aura LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(AURA_SANITIZE "Enable ASan+UBSan" OFF)

add_compile_options(-Wall -Wextra -Wpedantic)
if (AURA_SANITIZE)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
    add_link_options   (-fsanitize=address,undefined)
endif()

# --- Toolchain probe ---
find_package(FLEX   REQUIRED)
find_package(BISON  REQUIRED)
message(STATUS "Flex  ${FLEX_VERSION}")
message(STATUS "Bison ${BISON_VERSION}")

# --- LLVM probe ---
find_package(LLVM 17 REQUIRED CONFIG)
message(STATUS "LLVM  ${LLVM_PACKAGE_VERSION} @ ${LLVM_DIR}")

# --- Testing setup ---
find_package(GTest REQUIRED)
enable_testing()

# --- Main Library ---
add_library(aura_data STATIC
    src/data/aurd.cpp
    src/data/csv.cpp
)
target_include_directories(aura_data PUBLIC src)

# --- CLI Executable ---
add_executable(aura_cli src/cli/main.cpp)
target_link_libraries(aura_cli PRIVATE aura_data)

# --- Unit Tests ---
add_executable(aura_tests
    tests/unit/test_data.cpp
    tests/unit/test_csv.cpp
)
target_link_libraries(aura_tests PRIVATE aura_data GTest::gtest_main)

add_test(NAME aura_tests COMMAND aura_tests)