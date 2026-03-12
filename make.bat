@echo off

rem 清理之前的构建文件
echo Cleaning previous build files...
if exist CMakeCache.txt del CMakeCache.txt
if exist cmake_install.cmake del cmake_install.cmake
if exist Makefile del Makefile
if exist chatroom.exe del chatroom.exe
if exist chatroom.pdb del chatroom.pdb
if exist CMakeFiles rmdir /s /q CMakeFiles

rem 执行 cmake .
echo Running CMake...
cmake .

rem 执行 cmake --build .
echo Building project...
cmake --build .
