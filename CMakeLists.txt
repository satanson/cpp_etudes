cmake_minimum_required(VERSION 2.8.12)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmakes ${CMAKE_MODULE_PATH})

project(cpp_etudes)

enable_language(CXX)
enable_language(C)
enable_language(ASM)


execute_process(COMMAND which ccache OUTPUT_VARIABLE ccache_path)
message(STATUS "ccache: ${ccache_path}")

if (EXISTS ${ccache_path})
  message(STATUS "ccache_path:exist")
endif()

if (ccache_path)
  option(CCACHE_ENABLE "enable ccache" ON)
else()
  option(CCACHE_ENABLE "enable ccache" OFF)
endif()

if (CCACHE_ENABLE)
  message(STATUS "ccache enabled")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif()

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/third-party/gtest-1.7.0/fused-src)
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/third-party/benchmark-1.5.1/include)
add_subdirectory(third-party/gtest-1.7.0/fused-src/gtest)
add_subdirectory(third-party/benchmark-1.5.1)
add_subdirectory(src/modules)
message(STATUS "link_libs=${link_libs}")
add_subdirectory(unittest)
add_subdirectory(benchmark)
add_subdirectory(src/main)
