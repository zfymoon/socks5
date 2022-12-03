//
// Created by curie on 2022/11/29.
//

#ifndef SOCKS5_SERVER_HPP
#define SOCKS5_SERVER_HPP
#include <utility>

#include "string"
#include "unordered_map"
#include "./User.hpp"

using namespace std;

class Server {
public:
    string server {};
    int port {};
    User user {};
    Server(string  serverName, int port,User u)
        : server(std::move(serverName)),port(port),user(std::move(u)){}
    Server(){}
};


#endif //SOCKS5_SERVER_HPP
