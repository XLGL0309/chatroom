@echo off

rem 清理之前的构建文件
echo Cleaning previous build files...
call clean.bat

rem 执行 cmake .
echo Running CMake...
cmake .

rem 执行 cmake --build .
echo Building project...
cmake --build .

rem 构建完成后再次清理，只保留可执行文件
echo Cleaning up after build...
call clean.bat

echo Build completed successfully! Only chatroom.exe and chatroom.pdb remain.