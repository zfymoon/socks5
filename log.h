//
// Created by curie on 2022/1/7.
//

#ifndef SOCKS5_LOG_H
#define SOCKS5_LOG_H

#include <string>
#include <iostream>
using namespace std;
class log{
public:
    static void d(const string& tag,const string& msg){
        cout<<"["+tag<<"] "<<msg<<endl;
    }
    static void e(const string& tag,const string& msg){
        cout<<"["+tag<<"] "<<msg<<endl;
    }
};
#endif //SOCKS5_LOG_H
