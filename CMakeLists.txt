cmake_minimum_required(VERSION 2.8.12)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmakes ${CMAKE_MODULE_PATH})

project(cpp_etudes)
#enable_language(CXX)
#enable_language(C)
#enable_language(ASM)

set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS} -static-libstdc++  -msse4.1 -msse4.2 -mavx2 -mpopcnt -O3 -std=c++17")
set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS} -static-libstdc++ -msse4.1 -msse4.2 -mavx2 -g -O0 -std=c++17")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

include(cmakes/arch.cmake)
include(cmakes/target.cmake)
#include(cmakes/tools.cmake)
include(cmakes/analysis.cmake)
#include(cmakes/cpu_features.cmake)
option(LLVM_ENABLE "llvm enable" OFF)
if (LLVM_ENABLE)
  # clang: warning: argument unused during compilation: '-specs=/usr/share/dpkg/no-pie-compile.specs' [-Wunused-command-line-argument]
  set (COMMON_WARNING_FLAGS "${COMMON_WARNING_FLAGS} -Wno-unused-command-line-argument")
  # generate ranges for fast "addr2line" search
  if (NOT CMAKE_BUILD_TYPE_UC STREQUAL "RELEASE")
    set(COMPILER_FLAGS "${COMPILER_FLAGS} -gdwarf-aranges")
  endif ()

  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_CXX_STANDARD} -fsized-deallocation")

  set (CMAKE_BUILD_COLOR_MAKEFILE          ON)
  set (CMAKE_CXX_FLAGS                     "${CMAKE_CXX_FLAGS} ${COMPILER_FLAGS} ${PLATFORM_EXTRA_CXX_FLAG} ${COMMON_WARNING_FLAGS} ${CXX_WARNING_FLAGS}")
  set (CMAKE_CXX_FLAGS_RELWITHDEBINFO      "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O3 ${CMAKE_CXX_FLAGS_ADD}")
  set (CMAKE_CXX_FLAGS_DEBUG               "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3 -ggdb3 -fno-inline ${CMAKE_CXX_FLAGS_ADD}")

  set (CMAKE_C_FLAGS                       "${CMAKE_C_FLAGS} ${COMPILER_FLAGS} ${COMMON_WARNING_FLAGS} ${CMAKE_C_FLAGS_ADD}")
  set (CMAKE_C_FLAGS_RELWITHDEBINFO        "${CMAKE_C_FLAGS_RELWITHDEBINFO} -O3 ${CMAKE_C_FLAGS_ADD}")
  set (CMAKE_C_FLAGS_DEBUG                 "${CMAKE_C_FLAGS_DEBUG} -O0 -g3 -ggdb3 -fno-inline ${CMAKE_C_FLAGS_ADD}")

  if (OS_DARWIN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-U,_inside_main")
  endif()

  # Display absolute paths in error messages. Otherwise KDevelop fails to navigate to correct file and opens a new file instead.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-absolute-paths")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-absolute-paths")

  if (NOT ENABLE_TESTS AND NOT SANITIZE)
    # https://clang.llvm.org/docs/ThinLTO.html
    # Applies to clang only.
    # Disabled when building with tests or sanitizers.
    option(ENABLE_THINLTO "Clang-specific link time optimization" ON)
  endif()

  # We cannot afford to use LTO when compiling unit tests, and it's not enough
  # to only supply -fno-lto at the final linking stage. So we disable it
  # completely.
  if (ENABLE_THINLTO AND NOT ENABLE_TESTS AND NOT SANITIZE)
    # Link time optimization
    set (CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -flto=thin")
    set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -flto=thin")
    set (CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} -flto=thin")
  elseif (ENABLE_THINLTO)
    message (${RECONFIGURE_MESSAGE_LEVEL} "Cannot enable ThinLTO")
  endif ()

  # Always prefer llvm tools when using clang. For instance, we cannot use GNU ar when llvm LTO is enabled
  find_program (LLVM_AR_PATH NAMES "llvm-ar" "llvm-ar-11" "llvm-ar-10" "llvm-ar-9" "llvm-ar-8")

  if (LLVM_AR_PATH)
    message(STATUS "Using llvm-ar: ${LLVM_AR_PATH}.")
    set (CMAKE_AR ${LLVM_AR_PATH})
  else ()
    message(WARNING "Cannot find llvm-ar. System ar will be used instead. It does not work with ThinLTO.")
  endif ()

  find_program (LLVM_RANLIB_PATH NAMES "llvm-ranlib" "llvm-ranlib-11" "llvm-ranlib-10" "llvm-ranlib-9" "llvm-ranlib-8")

  if (LLVM_RANLIB_PATH)
    message(STATUS "Using llvm-ranlib: ${LLVM_RANLIB_PATH}.")
    set (CMAKE_RANLIB ${LLVM_RANLIB_PATH})
  else ()
    message(WARNING "Cannot find llvm-ranlib. System ranlib will be used instead. It does not work with ThinLTO.")
  endif ()
  include (cmakes/llvm.cmake)
endif()

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
