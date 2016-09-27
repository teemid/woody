@echo off

IF NOT EXIST build MKDIR build

if "%1" == "msvc" GOTO msvc
if "%1" == "clang" GOTO clang

:msvc
    SET COMPILER_FLAGS=/nologo /Od /W4 /wd4127 /wd4996 /Zi /Fobuild\ /Fdbuild\ /c /I "include"

    FOR /r %%f IN ("src\*.c") DO cl %COMPILER_FLAGS% "%%f" %MACROS%

    SET LINKER_FLAGS=/NOLOGO /DEBUG

    PUSHD build

    link %LINKER_FLAGS% /OUT:woody.exe *.obj

    POPD

    GOTO end

:clang
    REM -Werror - turn warnings into errors
    REM -g - generate complete debugging information
    REM -pedantic-errors - request error if a feature from a later standard is used in an earlier mode

    SET COMPILER_FLAGS=-Werror -g -pedantic-errors

    FOR /r %%f IN ("src/*.c") DO clang %COMPILER_FLAGS% "%%f" %MACROS%

    GOTO end

:end
    echo "Finished!"
