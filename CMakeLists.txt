cmake_minimum_required(VERSION 2.8.12)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmakes ${CMAKE_MODULE_PATH})

project(cpp_etudes)

enable_language(CXX)
enable_language(C)
enable_language(ASM)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/third-party/gtest-1.7.0/fused-src)
add_subdirectory(third-party/gtest-1.7.0/fused-src/gtest)
add_subdirectory(src/modules)
message(STATUS "link_libs=${link_libs}")
add_subdirectory(unittest)
add_subdirectory(src/main)
