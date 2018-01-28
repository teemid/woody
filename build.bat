@echo off

rem Generate VS Project

SET CMAKE_DIR=build

IF EXIST %CMAKE_DIR% GOTO build

MD %CMAKE_DIR%

:build
    PUSHD %CMAKE_DIR%

    cmake .. -G "Visual Studio 15 2017"

    POPD

    rem Build the actual project
    cmake --build %CMAKE_DIR% --config Debug
