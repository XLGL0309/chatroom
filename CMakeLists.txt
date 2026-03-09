cmake_minimum_required(VERSION 3.10)
project(ChatRoom)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ChatRoomServer main.cpp)

if(WIN32)
    target_link_libraries(ChatRoomServer ws2_32)
endif()