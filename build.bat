@echo off

IF NOT EXIST build MKDIR build

PUSHD build

cmake ..
msbuild woody.vcxproj

POPD
