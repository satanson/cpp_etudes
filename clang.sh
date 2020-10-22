env CC=clang CXX=clang++ cmake -G Ninja -H. -Bbuild_ninja_clang_sse -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE=ON
