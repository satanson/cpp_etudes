#!/bin/bash
set -e -o pipefail
basedir=$(cd $(dirname $(readlink -f ${BASH_SOURCE:-$0}));pwd)
cd ${basedir}

builddir=build_debug
[ -d ${builddir} ] && rm -fr ${builddir}
cmake -H. -B${builddir}  -DCMAKE_BUILD_TYPE=Debug -D CMAKE_CXX_FLAGS_DEBUG="-O0 -g"
cd ${builddir}
make
