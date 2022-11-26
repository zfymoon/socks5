//
// Created by curie on 2022/11/26.
//

#ifndef SOCKS5_USER_HPP
#define SOCKS5_USER_HPP

#include <utility>

#include "string"

using std::string;

class User {
public:
    string name;
    string token;
    User(string  nameStr,string  tokenStr):name(std::move(nameStr)),token(std::move(tokenStr)){}
};


#endif //SOCKS5_USER_HPP
