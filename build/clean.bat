@echo off

rem 清理构建文件
if exist CMakeCache.txt del CMakeCache.txt
if exist cmake_install.cmake del cmake_install.cmake
if exist Makefile del Makefile
if exist ALL_BUILD.vcxproj del ALL_BUILD.vcxproj
if exist ALL_BUILD.vcxproj.filters del ALL_BUILD.vcxproj.filters
if exist ZERO_CHECK.vcxproj del ZERO_CHECK.vcxproj
if exist ZERO_CHECK.vcxproj.filters del ZERO_CHECK.vcxproj.filters
if exist clean-all.vcxproj del clean-all.vcxproj
if exist clean-all.vcxproj.filters del clean-all.vcxproj.filters
if exist ChatRoom.sln del ChatRoom.sln
if exist chatroom.vcxproj del chatroom.vcxproj
if exist chatroom.vcxproj.filters del chatroom.vcxproj.filters
if exist CMakeFiles rmdir /s /q CMakeFiles
if exist ALL_BUILD.dir rmdir /s /q ALL_BUILD.dir
if exist clean-all.dir rmdir /s /q clean-all.dir
if exist ZERO_CHECK.dir rmdir /s /q ZERO_CHECK.dir
if exist chatroom.dir rmdir /s /q chatroom.dir
if exist x64 rmdir /s /q x64
if exist Debug rmdir /s /q Debug
if exist chatroom.pdb del chatroom.pdb