#!/bin/bash
set -e -o pipefail
cmake -H. -Bdebug-build  -DCMAKE_BUILD_TYPE=Debug -D CMAKE_CXX_FLAGS_DEBUG="-O0 -g"
