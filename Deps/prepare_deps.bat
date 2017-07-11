@echo off

REM LKTODO:
REM  * Discover CPU count
REM  * Discover CWD, navigate to script's dir and run from there

pushd .

REM ========== glslang build ===========
cd glslang

mkdir build
cd build
mkdir x86
mkdir x64

cd x86
cmake ..\.. -DCMAKE_INSTALL_PREFIX=%cd% -DENABLE_HLSL=OFF -G "Visual Studio 15 2017"
msbuild /property:Configuration=Debug /maxcpucount:8 INSTALL.vcxproj
msbuild /property:Configuration=Release /maxcpucount:8 INSTALL.vcxproj

cd ..\x64
cmake ..\.. -DCMAKE_INSTALL_PREFIX=%cd% -DENABLE_HLSL=OFF -G "Visual Studio 15 2017 Win64"
msbuild /property:Configuration=Debug /maxcpucount:8 INSTALL.vcxproj
msbuild /property:Configuration=Release /maxcpucount:8 INSTALL.vcxproj

echo.
echo Script is done
echo.

popd
