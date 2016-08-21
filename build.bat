@echo off

IF NOT EXIST build MKDIR build

SET COMPILER_FLAGS=/nologo /Od /W4 /wd4127 /wd4996 /Zi /Fobuild\ /Fdbuild\ /c /I "include"

rem FOR /r %%f IN ("src\*.c") DO cl %COMPILER_FLAGS% "%%f" %MACROS%
cl %COMPILER_FLAGS% "src\woody.c" %MACROS%

SET LINKER_FLAGS=/NOLOGO /DEBUG

PUSHD build

link %LINKER_FLAGS% /OUT:woody.exe *.obj

POPD
