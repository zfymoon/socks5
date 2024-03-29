//
// Created by curie on 2022/1/10.
// server
//

#ifndef SOCKS5_SOCKS5SERVER_HPP
#define SOCKS5_SOCKS5SERVER_HPP

#include "asio.hpp"
#include "../common/Log.hpp"
#include "Socks5SessionV2.hpp"
#include "forward/ForwardSession.hpp"
#include "atomic"
#include "memory"
#include "./config/Config.hpp"
#include <unordered_map>

using asio::ip::tcp;
using namespace std;
using namespace asio;
using std::atomic_bool;
using std::unique_ptr;



class Socks5Server: public enable_shared_from_this<Socks5Server>{
private:
    const string TAG = "Socks5Server";
    int _sessionID;
    shared_ptr<Config> _config;
    io_context _context;
    tcp::acceptor _acceptor;
    unordered_map<int,weak_ptr<Socks5SessionV2>> _session_map;
    asio::thread_pool _taskPool;
    void start(){
        auto clientSocket = make_shared<tcp::socket>(_context);
        _acceptor.async_accept(
                *clientSocket,
                [ clientSocket,
                  self = shared_from_this() ]
                  (asio::error_code code){
                    if(code){
                        Log::error( self->TAG,"Receive connect error {}", code.message());
                    } else{
                        try {
                            int id = self->_sessionID++;
                            if(self->_config->forwardEnable) {
                                auto sessionInstance = make_shared<ForwardSession>(clientSocket, id, self->_config);
                                self->_session_map[id] = sessionInstance;
                                asio::post(self->_taskPool, [ sessionInstance ](){
                                    sessionInstance->start();
                                });
                            }else{
                                auto sessionInstance = make_shared<Socks5SessionV2>(clientSocket, id, self->_config);
                                self->_session_map[id] = sessionInstance;
                                asio::post(self->_taskPool, [ sessionInstance ](){
                                    sessionInstance->start();
                                });
                            }

                        }catch (exception &e){
                            Log::error( self->TAG,"[start] error "+string(e.what()));
                        }
                    }
                    self->start();
                });
    }
public:
    explicit Socks5Server(const string &configFile):
            _sessionID(0),
            _config(make_shared<Config>(configFile)),
            _context(),
            _acceptor(
                    _context,
                    tcp::endpoint(tcp::v4(),_config->port)),
            _taskPool(5){

    }

    void run(){
        string serverType = "Socks5";
        if(_config->forwardEnable){
            serverType = "Forward";
        }
        Log::info(TAG, "{} Server running at {}:{} ",
                  serverType,
                  _acceptor.local_endpoint().address().to_string(),
                  _acceptor.local_endpoint().port());
        start();
        _context.run();
    }
    ~Socks5Server(){
        _session_map.clear();
    }

};




#endif //SOCKS5_SOCKS5SERVER_HPP
