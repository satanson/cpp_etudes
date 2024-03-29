cmake_minimum_required(VERSION 2.8.12)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmakes ${CMAKE_MODULE_PATH})

project(cpp_etudes)
#enable_language(CXX)
#enable_language(C)
#enable_language(ASM)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -static-libstdc++  -msse4.1 -msse4.2 -mavx2 -mpopcnt -O3 -std=c++17")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -static-libstdc++ -msse4.1 -msse4.2 -mavx2 -g -O0 -std=c++17")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS}  -static-libstdc++  -O3 -std=c++17")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS}  -static-libstdc++ -g -O0 -std=c++17")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
include(cmakes/arch.cmake)
include(cmakes/target.cmake)
#include(cmakes/tools.cmake)
include(cmakes/analysis.cmake)
include(cmakes/llvm.cmake)
#include(cmakes/cpu_features.cmake)
option(LLVM_ENABLE "llvm enable" ON)
#if (LLVM_ENABLE)
    # clang: warning: argument unused during compilation: '-specs=/usr/share/dpkg/no-pie-compile.specs' [-Wunused-command-line-argument]
    set(COMMON_WARNING_FLAGS "${COMMON_WARNING_FLAGS} -Wno-unused-command-line-argument")
    # generate ranges for fast "addr2line" search
    #if (NOT CMAKE_BUILD_TYPE_UC STREQUAL "RELEASE")
    #    set(COMPILER_FLAGS "${COMPILER_FLAGS} -gdwarf-aranges")
    #endif ()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_CXX_STANDARD} -fsized-deallocation")

    set(CMAKE_BUILD_COLOR_MAKEFILE ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_FLAGS} ${PLATFORM_EXTRA_CXX_FLAG} ${COMMON_WARNING_FLAGS} ${CXX_WARNING_FLAGS}")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O3 ${CMAKE_CXX_FLAGS_ADD}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3 -ggdb3 -fno-inline ${CMAKE_CXX_FLAGS_ADD}")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILER_FLAGS} ${COMMON_WARNING_FLAGS} ${CMAKE_C_FLAGS_ADD}")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -O3 ${CMAKE_C_FLAGS_ADD}")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g3 -ggdb3 -fno-inline ${CMAKE_C_FLAGS_ADD}")

    if (OS_DARWIN)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-U,_inside_main")
    endif ()

    # Display absolute paths in error messages. Otherwise KDevelop fails to navigate to correct file and opens a new file instead.
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-absolute-paths")
    # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-absolute-paths")

    if (NOT ENABLE_TESTS AND NOT SANITIZE)
        # https://clang.llvm.org/docs/ThinLTO.html
        # Applies to clang only.
        # Disabled when building with tests or sanitizers.
        option(ENABLE_THINLTO "Clang-specific link time optimization" ON)
    endif ()

    # We cannot afford to use LTO when compiling unit tests, and it's not enough
    # to only supply -fno-lto at the final linking stage. So we disable it
    # completely.
    if (ENABLE_THINLTO AND NOT ENABLE_TESTS AND NOT SANITIZE)
        # Link time optimization
        set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -flto=thin")
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -flto=thin")
        set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} -flto=thin")
    elseif (ENABLE_THINLTO)
        message(${RECONFIGURE_MESSAGE_LEVEL} "Cannot enable ThinLTO")
    endif ()

    # Always prefer llvm tools when using clang. For instance, we cannot use GNU ar when llvm LTO is enabled
    find_program(LLVM_AR_PATH NAMES "llvm-ar" "llvm-ar-11" "llvm-ar-10" "llvm-ar-9" "llvm-ar-8")

    if (LLVM_AR_PATH)
        message(STATUS "Using llvm-ar: ${LLVM_AR_PATH}.")
        set(CMAKE_AR ${LLVM_AR_PATH})
    else ()
        message(WARNING "Cannot find llvm-ar. System ar will be used instead. It does not work with ThinLTO.")
    endif ()

    find_program(LLVM_RANLIB_PATH NAMES "llvm-ranlib" "llvm-ranlib-11" "llvm-ranlib-10" "llvm-ranlib-9" "llvm-ranlib-8")

    if (LLVM_RANLIB_PATH)
        message(STATUS "Using llvm-ranlib: ${LLVM_RANLIB_PATH}.")
        set(CMAKE_RANLIB ${LLVM_RANLIB_PATH})
    else ()
        message(WARNING "Cannot find llvm-ranlib. System ranlib will be used instead. It does not work with ThinLTO.")
    endif ()
    message(STATUS "LLVM include Directory: ${LLVM_INCLUDE_DIRS}")
    message(STATUS "LLVM library Directory: ${LLVM_LIBRARY_DIRS}")
    message(STATUS "LLVM C++ compiler flags: ${LLVM_CXXFLAGS}")
#endif ()

execute_process(COMMAND which ccache OUTPUT_VARIABLE ccache_path)
message(STATUS "ccache: ${ccache_path}")

if (EXISTS ${ccache_path})
    message(STATUS "ccache_path:exist")
endif ()

if (ccache_path)
    option(CCACHE_ENABLE "enable ccache" ON)
else ()
    option(CCACHE_ENABLE "enable ccache" OFF)
endif ()

if (CCACHE_ENABLE)
    message(STATUS "ccache enabled")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif ()

set(Boost_DEBUG FALSE)
set(Boost_USE_MULTITHREADED ON)
set(Boost_NO_BOOST_CMAKE ON)
set(BOOST_ROOT "${PROJECT_SOURCE_DIR}/boost_install/usr/local/")
# boost suppress warning is supported on cmake 3.20
# https://cmake.org/cmake/help/latest/module/FindBoost.html
set(Boost_NO_WARN_NEW_VERSIONS ON)

if (NOT APPLE)
    find_package(Boost 1.81.0 REQUIRED COMPONENTS thread filesystem date_time program_options)
else()
    find_package(Boost 1.81.0 COMPONENTS thread regex filesystem system date_time program_options)
endif()
include_directories(${Boost_INCLUDE_DIRS})
message(STATUS "boost=${Boost_LIBRARIES}")

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src/modules)
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/third-party/gtest-1.7.0/fused-src)
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/third-party/benchmark-1.5.1/include)
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/third-party/glog-0.6.0/src)
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/third-party/abseil-cpp-91b7cd600a34cbd/)

add_subdirectory(third-party/gtest-1.7.0/fused-src/gtest)
add_subdirectory(third-party/benchmark-1.5.1)
set(BUILD_TESTING OFF)
add_subdirectory(third-party/glog-0.6.0)

set(ABSL_PROPAGATE_CXX_STD ON)
set(ABSL_BUILD_TESTING OFF)
add_subdirectory(third-party/abseil-cpp-91b7cd600a34cbd)
set(BUILD_TESTING ON)

include (CheckCXXSourceCompiles)
check_cxx_source_compiles("
	#include <iostream>
	int main() {
#if !_GLIBCXX_USE_CXX11_ABI
#error _GLIBCXX_USE_CXX11_ABI must be non-zero
#endif
	   return 0;
	}
" HAVE_GLIBCXX_USE_CXX11_ABI)
if (NOT HAVE_GLIBCXX_USE_CXX11_ABI)
    MESSAGE(FATAL_ERROR "_GLIBCXX_USE_CXX11_ABI should not be 0")
endif()

list(APPEND link_libs gtest
        pthread atomic
        glog::glog
        absl::algorithm
        absl::base
        absl::debugging
        absl::flat_hash_map
        absl::flags
        absl::memory
        absl::meta
        absl::numeric
        absl::random_random
        absl::status
        absl::statusor
        absl::strings
        absl::synchronization
        absl::time
        absl::utility
        ${Boost_LIBRARIES}
        ${REQUIRED_LLVM_LIBRARIES}
        )
list(APPEND link_libs curses)
add_subdirectory(src/modules)
message(STATUS "link_libs=${link_libs}")
add_subdirectory(unittest)
add_subdirectory(benchmark)
add_subdirectory(src/main)

SET(IN_DOCKER_ENV OFF)
FIND_FILE(DOCKER_ENV NAMES ".dockerenv" PATHS "/" NO_CACHE)
if (NOT (DOCKER_ENV STREQUAL "DOCKER_ENV-NOTFOUND"))
    SET(IN_DOCKER_ENV ON)
endif()


if (IN_DOCKER_ENV)
MESSAGE(STATUS "IN_DOCKER_ENV=${IN_DOCKER_ENV}")
endif()