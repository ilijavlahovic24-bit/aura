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

# Flex generates lexer.cpp from lexer.l
FLEX_TARGET(AuraLexer
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lang/lexer.l
    ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp
)

# Bison generates aura_parser.cpp / aura_parser.hpp from parser.y
BISON_TARGET(AuraParser
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lang/parser.y
    ${CMAKE_CURRENT_BINARY_DIR}/aura_parser.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/aura_parser.hpp
)

# --- Language library (single definition) ---
add_library(aura_lang STATIC
    src/lang/token.cpp
    src/lang/type.cpp
    src/lang/ast.cpp
    src/lang/sema.cpp
    src/lang/ir.cpp
    src/lang/ir_builder.cpp 
    ${FLEX_AuraLexer_OUTPUTS}
    ${BISON_AuraParser_OUTPUTS}
)
target_include_directories(aura_lang
    PUBLIC  src
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)

# Lexer driver
add_executable(aura_lex src/lang/lexer_main.cpp)
target_link_libraries(aura_lex PRIVATE aura_lang)
target_include_directories(aura_lex PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# Parser driver
add_executable(aura_parse src/lang/parser_main.cpp)
target_link_libraries(aura_parse PRIVATE aura_lang)
target_include_directories(aura_parse PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
# IR driver
add_executable(aura_ir src/lang/ir_main.cpp)
target_link_libraries(aura_ir PRIVATE aura_lang)

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
    src/data/gorilla.cpp
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