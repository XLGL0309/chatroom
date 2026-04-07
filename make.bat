@echo off

rem 创建并进入 build 目录
if not exist build mkdir build
cd build

rem 执行 cmake ..
echo Running CMake...
cmake ..

rem 执行构建
echo Building project...
cmake --build .

rem 回到根目录
cd ..

echo Build completed successfully!