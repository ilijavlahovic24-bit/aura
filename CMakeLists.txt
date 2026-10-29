cmake_minimum_required(VERSION 3.22)
project(aura LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(AURA_SANITIZE "Enable ASan+UBSan" OFF)

add_compile_options(-Wall -Wextra -Wpedantic)
if (AURA_SANITIZE)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
    add_link_options   (-fsanitize=address,undefined)
endif()

# --- toolchain probe  ---
find_package(FLEX   REQUIRED)
find_package(BISON  REQUIRED)
message(STATUS "Flex  ${FLEX_VERSION}")
message(STATUS "Bison ${BISON_VERSION}")

# --- LLVM probe  ---
find_package(LLVM 17 REQUIRED CONFIG)
message(STATUS "LLVM  ${LLVM_PACKAGE_VERSION} @ ${LLVM_DIR}")

# --- placeholder target for build ---
add_library(aura_data STATIC src/data/aurd.cpp)
target_include_directories(aura_data PUBLIC src)

add_executable(aura_cli src/cli/main.cpp)
target_link_libraries(aura_cli PRIVATE aura_data)