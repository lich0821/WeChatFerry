cmake_minimum_required(VERSION 3.15)

# Common compiler flags
# 设置 C 语言标准为 C17
set(CMAKE_C_STANDARD 17)
# 确保要求该标准为必须
# set(CMAKE_C_STANDARD_REQUIRED ON)
# 默认情况下禁用 GNU 扩展
set(CMAKE_C_EXTENSIONS OFF)
# 设置 C++ 语言标准为 C++17
set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 集成vcpkg
set(VCPKG_TARGET_TRIPLET "x64-mingw-static" CACHE STRING "Vcpkg target triplet")
set(VCPKG_HOST_TRIPLET "x64-mingw-static" CACHE STRING "Vcpkg host triplet")
set(VCPKG_MANIFEST_MODE ON CACHE BOOL "Enable manifest mode")

if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
else()
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

# include(FetchContent)
# include(ExternalProject)

project(WeChatFerry LANGUAGES C CXX)

add_compile_options(
    -Wall
    -Wextra
    -fPIC
)

add_link_options(
    -static
)

# Include directories
include_directories(
  ${CMAKE_SOURCE_DIR}/com
  ${CMAKE_SOURCE_DIR}/rpc
  ${CMAKE_SOURCE_DIR}/rpc/nanopb
  ${CMAKE_SOURCE_DIR}/rpc/proto
  ${CMAKE_SOURCE_DIR}/sdk
  ${CMAKE_SOURCE_DIR}/spy
  ${CMAKE_SOURCE_DIR}/smc
)

# Add subdirectories

add_subdirectory(sdk)
add_subdirectory(spy)

