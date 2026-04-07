cmake_minimum_required(VERSION 3.10)
project(ChatRoom)

# 指定使用 MSVC 编译器
if(MSVC)
    message(STATUS "Using MSVC compiler")
    # 确保使用 C++11 标准
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    # 禁用某些 MSVC 警告
    add_compile_options(/W3 /wd4996)
endif()

# 设置可执行文件输出目录为根目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# 对于 MSVC，确保 Debug 和 Release 版本都输出到根目录
if(MSVC)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR})
    # 设置调试符号文件输出目录到 build 文件夹
    set(CMAKE_PDB_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
    set(CMAKE_PDB_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})
endif()

include_directories(include)

# MySQL C API 依赖
# 使用用户提供的实际安装路径
if(WIN32)
    # Windows 平台
    include_directories("E:/MySQL/MySQL Server 8.0/include")
    link_directories("E:/MySQL/MySQL Server 8.0/lib")
else()
    # Linux 平台
    include_directories(/usr/include/mysql)
    link_directories(/usr/lib64/mysql)
endif()
# 收集src 目录下的 .cpp 文件
file(GLOB_RECURSE SRC_FILES "src/*.cpp")
add_executable(chatroom ${SRC_FILES})

# 跨平台链接socket/线程库
if(WIN32)
    target_link_libraries(chatroom ws2_32 libmysql)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(chatroom Threads::Threads mysql)
endif()