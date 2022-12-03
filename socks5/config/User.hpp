//
// Created by curie on 2022/11/26.
//

#ifndef SOCKS5_USER_HPP
#define SOCKS5_USER_HPP

#include <utility>

#include "string"

using std::string;
const int USER_TOKEN_MIN = 10;
class User {
public:
    string name {};
    string token {};
    User(string  nameStr,string  tokenStr):name(std::move(nameStr)),token(std::move(tokenStr)){}
    User(){}
};


#endif //SOCKS5_USER_HPP
