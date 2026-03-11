cmake_minimum_required(VERSION 3.10)
project(ChatRoom)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(chatroom
    main.cpp
    src/network.cpp
    src/message.cpp
    src/user.cpp
    src/web.cpp
    src/utils.cpp
)

# 跨平台链接socket/线程库
if(WIN32)
    target_link_libraries(chatroom ws2_32)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(chatroom Threads::Threads)
endif()