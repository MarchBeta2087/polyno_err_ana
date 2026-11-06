cmake_minimum_required(VERSION 3.20)
project(polyno_err_ana VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "" FORCE)
endif()

set(NEX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/numextend)

# 优先复用已构建的 libnex.a；否则作为子工程构建。
find_library(NEX_LIB NAMES nex PATHS ${NEX_DIR}/build NO_DEFAULT_PATH)
if(NOT NEX_LIB)
    add_subdirectory(${NEX_DIR} ${CMAKE_BINARY_DIR}/numextend)
    set(NEX_LIB nex)
    message(STATUS "numextend: 作为子工程构建")
else()
    message(STATUS "numextend: 复用 ${NEX_LIB}")
endif()

option(POLYNO_ENABLE_MPFR "使用 MPFR 内核（需 third_party_prefix 或系统 GMP/MPFR）" OFF)
set(PNA_THIRD_PARTY "${CMAKE_CURRENT_SOURCE_DIR}/third_party_prefix" CACHE PATH "GMP/MPFR 安装前缀")

set(PNA_SOURCES src/main.cpp src/ast.cpp src/sym.cpp src/num_common.cpp)

if(POLYNO_ENABLE_MPFR)
    find_path(MPFR_INCLUDE_DIR mpfr.h HINTS ${PNA_THIRD_PARTY}/include)
    find_path(GMP_INCLUDE_DIR gmp.h HINTS ${PNA_THIRD_PARTY}/include)
    find_library(MPFR_LIBRARY mpfr HINTS ${PNA_THIRD_PARTY}/lib)
    find_library(GMP_LIBRARY gmp HINTS ${PNA_THIRD_PARTY}/lib)
    if(NOT MPFR_INCLUDE_DIR OR NOT MPFR_LIBRARY OR NOT GMP_LIBRARY)
        message(FATAL_ERROR "未找到 GMP/MPFR：请先构建到 ${PNA_THIRD_PARTY}（见 README）")
    endif()
    list(APPEND PNA_SOURCES src/num_mpfr.cpp)
    set(PNA_KERNEL_DEF POLYNO_KERNEL_MPFR=1)
    message(STATUS "内核: MPFR (${MPFR_LIBRARY})")
else()
    list(APPEND PNA_SOURCES src/num.cpp)
    set(PNA_KERNEL_DEF POLYNO_KERNEL_NUMEXTEND=1)
    message(STATUS "内核: numextend")
endif()

# -G 用：把 LICENSE.md 转成常量字符串头
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md PNA_LICENSE_TEXT)
configure_file(cmake/license_text.hpp.in ${CMAKE_BINARY_DIR}/generated/license_text.hpp @ONLY)

add_executable(polyno_err_ana ${PNA_SOURCES})
target_compile_definitions(polyno_err_ana PRIVATE ${PNA_KERNEL_DEF})
target_include_directories(polyno_err_ana PRIVATE ${NEX_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/generated)
if(POLYNO_ENABLE_MPFR)
    target_include_directories(polyno_err_ana PRIVATE ${MPFR_INCLUDE_DIR} ${GMP_INCLUDE_DIR})
    target_link_libraries(polyno_err_ana PRIVATE ${MPFR_LIBRARY} ${GMP_LIBRARY})
endif()
target_link_libraries(polyno_err_ana PRIVATE ${NEX_LIB})

enable_testing()
find_program(BASH_EXE bash)
if(BASH_EXE)
    add_test(NAME regression
             COMMAND ${BASH_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/checks.sh $<TARGET_FILE:polyno_err_ana>)
    add_test(NAME snapshot
             COMMAND ${BASH_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/snapshot_test.sh $<TARGET_FILE:polyno_err_ana>)
endif()
