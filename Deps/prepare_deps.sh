#!/bin/bash
set -e

pushd . > /dev/null

# ========== glslang build ===========
cd glslang

mkdir -p build
cd build
mkdir -p x86_64

cd x86_64
cmake ../.. -DCMAKE_INSTALL_PREFIX="`pwd`/../.." -DENABLE_HLSL=OFF
make && make install

echo -n
echo "Script is done"
echo -n

popd > /dev/null
