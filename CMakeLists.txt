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

add_executable(chatroom
    main.cpp
    src/network.cpp
    src/message.cpp
    src/user.cpp
    src/web.cpp
    src/utils.cpp
    src/database.cpp
)

# 跨平台链接socket/线程库
if(WIN32)
    target_link_libraries(chatroom ws2_32 libmysql)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(chatroom Threads::Threads mysql)
endif()

# 添加清理目标，删除构建生成的文件
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove -f chatroom.exe chatroom.pdb
    COMMAND ${CMAKE_COMMAND} -E remove_directory CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove -f CMakeCache.txt cmake_install.cmake Makefile
    COMMENT "Cleaning all build files"
)