@echo off
echo Building project files for eclipse-msw
echo Note that there are 2 projects, one in the root dir that contains the source, and one in build\ that contains the actual builds
mkdir build\eclipse-msw
copy lib\msw\*.dll build\eclipse-msw
echo Deleting cache and re-running cmake
cd build\eclipse-msw
del CMakeCache.txt
cmake -G "Eclipse CDT4 - MinGW Makefiles" -DECLIPSE_CDT4_GENERATE_SOURCE_PROJECT=TRUE -DCMAKE_BUILD_TYPE:STRING=Debug ..\..
cd ..\..
echo Done!

