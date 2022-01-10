//
// Created by curie on 2022/1/10.
// server
//

#ifndef SOCKS5_SERVER_H
#define SOCKS5_SERVER_H

#include "libs/asio/include/asio.hpp"
#include "log.h"
#include "session.h"
#include <unordered_map>

using asio::ip::tcp;
using namespace std;
using namespace asio;

class server {

private:
    const string TAG = "server";
    int _session_id = 0;
    io_context _context;
    tcp::acceptor _acceptor;
    tcp::socket _socket;
    unordered_map<int,weak_ptr<session>> _session_map;

    void start(){
        _acceptor.async_accept(_socket, [this](asio::error_code code){
            try {
                _session_id += 1;
                auto session_obj = std::make_shared<session>(move(_socket), _session_id);
                session_obj->start();
                _session_map[_session_id] = std::weak_ptr(session_obj);
                start();
            }catch (exception &e){
                log::e(TAG,"[start] error "+string(e.what()));
            }
        });
    }
public:
    explicit server(int port):_context(),
                              _acceptor(_context,
                                        tcp::endpoint( tcp::v4(), port)),
                              _socket(_context){

    }
    void run(){
        log::d(TAG,"server run");
        start();
        _context.run();
    }
};




#endif //SOCKS5_SERVER_H
