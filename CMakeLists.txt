cmake_minimum_required(VERSION 3.16)
project(socks5)

set(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DASIO_STANDALONE -pthread")
aux_source_directory(. SRC_LIST)
include_directories(libs/asio/include)

add_executable(socks5 main.cpp session.h log.h server.h)
