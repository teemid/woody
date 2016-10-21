@echo off

rem Generate VS Project

SET CMAKE_DIR=_cmake

IF EXIST %CMAKE_DIR% GOTO build

MD %CMAKE_DIR%

:build
    PUSHD %CMAKE_DIR%

    cmake .. -G "Visual Studio 14 2015"

    POPD

    rem Build the actual project
    cmake --build %CMAKE_DIR% --config Debug


rem Old build: cl /nologo /Od /W4 /wd4127 /wd4996 /Zi /Fobuild\ /Fdbuild\ /c /I "include"
