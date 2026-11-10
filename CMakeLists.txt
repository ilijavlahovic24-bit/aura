cmake_minimum_required(VERSION 3.22)
project(aura LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(AURA_SANITIZE "Enable ASan+UBSan" OFF)
add_compile_options(-Wall -Wextra -Wpedantic)
if (AURA_SANITIZE)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address,undefined)
endif()

# --- Toolchain ---
find_package(FLEX  REQUIRED)
find_package(BISON REQUIRED)
find_package(LLVM 17 REQUIRED CONFIG)
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
message(STATUS "Flex  ${FLEX_VERSION}")
message(STATUS "Bison ${BISON_VERSION}")
message(STATUS "LLVM  ${LLVM_PACKAGE_VERSION} @ ${LLVM_DIR}")

include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# --- Generisani ---
FLEX_TARGET(AuraLexer
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lang/lexer.l
    ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
BISON_TARGET(AuraParser
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lang/parser.y
    ${CMAKE_CURRENT_BINARY_DIR}/aura_parser.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/aura_parser.hpp)

# --- Biblioteke ---
add_library(aura_data STATIC
    src/data/aurd.cpp src/data/csv.cpp src/data/gorilla.cpp)
target_include_directories(aura_data PUBLIC src)

add_library(aura_lang STATIC
    src/lang/token.cpp src/lang/type.cpp src/lang/ast.cpp
    src/lang/sema.cpp src/lang/ir.cpp src/lang/ir_builder.cpp
    ${FLEX_AuraLexer_OUTPUTS} ${BISON_AuraParser_OUTPUTS})
target_include_directories(aura_lang PUBLIC src PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

add_library(aura_rt STATIC src/rt/kernels.cpp src/rt/extern_c.cpp)
target_include_directories(aura_rt PUBLIC src)
target_compile_options(aura_rt PRIVATE -mavx2)

add_library(aura_sim STATIC src/sim/simulator.cpp)
target_include_directories(aura_sim PUBLIC src)

add_library(aura_codegen STATIC
    src/codegen/llvm_codegen.cpp
    src/codegen/jit.cpp
    src/codegen/interp.cpp)
target_include_directories(aura_codegen PUBLIC src ${LLVM_INCLUDE_DIRS})
target_link_libraries(aura_codegen PUBLIC aura_lang aura_rt)

llvm_map_components_to_libnames(LLVM_LIBS
    core irreader support orcjit executionengine native nativecodegen)
target_link_libraries(aura_codegen PUBLIC ${LLVM_LIBS})

add_library(aura_server STATIC src/server/http_server.cpp)
target_include_directories(aura_server PUBLIC src)
target_link_libraries(aura_server PUBLIC Threads::Threads)

# --- Alati ---
add_executable(aura_lex   src/lang/lexer_main.cpp)
target_link_libraries(aura_lex PRIVATE aura_lang)
target_include_directories(aura_lex PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

add_executable(aura_parse src/lang/parser_main.cpp)
target_link_libraries(aura_parse PRIVATE aura_lang)
target_include_directories(aura_parse PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

add_executable(aura_ir    src/lang/ir_main.cpp)
target_link_libraries(aura_ir PRIVATE aura_lang)

add_executable(aura_emit  src/codegen/codegen_main.cpp)
target_link_libraries(aura_emit PRIVATE aura_codegen)

add_executable(aura_run   src/codegen/jit_main.cpp)
target_link_libraries(aura_run PRIVATE
    aura_codegen aura_sim aura_rt aura_data Threads::Threads)
target_link_options(aura_run PRIVATE -rdynamic)
target_link_options(aura_run PRIVATE
    "-Wl,--whole-archive" "$<TARGET_FILE:aura_rt>" "-Wl,--no-whole-archive")

# --- Glavni CLI ---
add_executable(aura_cli
    src/cli/main.cpp
    src/cli/cmd.cpp
    src/cli/cmd_import.cpp
    src/cli/cmd_info.cpp
    src/cli/cmd_check.cpp
    src/cli/cmd_run.cpp
    src/cli/cmd_sweep.cpp
    src/cli/cmd_serve.cpp)
target_link_libraries(aura_cli PRIVATE
    aura_data aura_lang aura_codegen aura_sim aura_rt aura_server
    Threads::Threads)
target_link_options(aura_cli PRIVATE -rdynamic)
target_link_options(aura_cli PRIVATE
    "-Wl,--whole-archive" "$<TARGET_FILE:aura_rt>" "-Wl,--no-whole-archive")

# --- Testovi ---
enable_testing()
add_executable(aura_tests
    tests/unit/test_data.cpp
    tests/unit/test_csv.cpp)
target_link_libraries(aura_tests PRIVATE aura_data GTest::gtest_main)
add_test(NAME aura_tests COMMAND aura_tests)