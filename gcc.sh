#!/bin/bash
rm -fr build_ninja_gcc_sse
cmake -G Ninja -H. -Bbuild_ninja_gcc_sse -DLLVM_ENABLE=off -DCMAKE_BUILD_TYPE=Release
ninja -j 12 -C build_ninja_gcc_sse
