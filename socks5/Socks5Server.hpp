//
// Created by curie on 2022/1/10.
// server
//

#ifndef SOCKS5_SOCKS5SERVER_HPP
#define SOCKS5_SOCKS5SERVER_HPP

#include "asio.hpp"
#include "../common/Log.hpp"
#include "Socks5SessionV2.hpp"
#include "atomic"
#include "memory"
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
                            auto sessionInstance = make_shared<Socks5SessionV2>(clientSocket,id);
                            self->_session_map[id] = sessionInstance;
                            asio::post(self->_taskPool, [ sessionInstance ](){
                                sessionInstance->start();
                            });
                        }catch (exception &e){
                            Log::error( self->TAG,"[start] error "+string(e.what()));
                        }
                    }
                    self->start();
                });
    }
public:
    explicit Socks5Server(int port):
            _sessionID(0),
            _context(),
            _acceptor(
                    _context,
                    tcp::endpoint(tcp::v4(),port)),
            _taskPool(5)
                    {}
    void run(){
        Log::info(TAG,"Server running at {}:{} ",
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
