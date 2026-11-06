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

add_executable(polyno_err_ana
    src/main.cpp src/num.cpp src/ast.cpp src/sym.cpp)
target_include_directories(polyno_err_ana PRIVATE ${NEX_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(polyno_err_ana PRIVATE ${NEX_LIB})

enable_testing()
find_program(BASH_EXE bash)
if(BASH_EXE)
    add_test(NAME regression
             COMMAND ${BASH_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/checks.sh $<TARGET_FILE:polyno_err_ana>)
endif()
