cmake_minimum_required(VERSION 3.13)
project(socks5)

set(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -DASIO_STANDALONE -pthread")
aux_source_directory(. SRC_LIST)
include_directories(libs/asio/include)
include_directories(libs/spdlog/include)

add_executable(socks5 main.cpp
        socks5/Socks5Server.hpp common/Log.hpp common/FileUtil.hpp
        socks5/DataHandler.hpp socks5/Socks5SessionV2.hpp
        util/NetUtil.hpp
        socks5/SocksAddr.h
        socks5/Socks5State.hpp
        socks5/client/SocketClient.hpp
        socks5/config/Config.hpp
        socks5/config/User.hpp
        socks5/forward/ForwardSession.hpp
        socks5/config/Server.hpp
        socks5/forward/Socks5Client.hpp)
