//
// Created by curie on 2022/11/13.
//

#ifndef SOCKS5_SOCKETCLIENT_HPP
#define SOCKS5_SOCKETCLIENT_HPP
#include <utility>

#include "../SocksAddr.h"
#include "asio.hpp"
#include "memory"
#include "chrono"
#include "../DataHandler.hpp"

using std::shared_ptr;
using asio::ip::tcp;
using std::string;
using ConnectResult = std::function<void (bool,tcp::endpoint)>;
/**
 * Socket链接，负责连接服务器和读写数据
 */
class SocketClient: public DataHandler{
private:
    string TAG = "SocketClient";
    string _serverName{};
    int _port {};
    tcp::resolver _resolver;
public:
    SocketClient(const shared_ptr<SocksAddr>& addr, shared_ptr<tcp::socket> socket)
                : _serverName(addr->getAddrName()),
                  _port(addr->getPort()),
                  _resolver(getSocket()->get_executor()),
                  DataHandler(std::move(socket)){}
    SocketClient(string  addr,int port,shared_ptr<tcp::socket> socket)
                : _resolver(getSocket()->get_executor()),
                  _serverName(std::move(addr)),
                  _port(port),
                  DataHandler(std::move(socket)){}
    void connect(const ConnectResult& result){
        Log::info(TAG,
                   "Start connect to {}:{} ",
                   _serverName,
                   std::to_string(_port));
        auto currentTime = std::chrono::system_clock::now();
        _resolver.async_resolve(
                tcp::resolver::query( _serverName, std::to_string(_port)),
                [self = self<SocketClient>(),result,currentTime](asio::error_code code, const tcp::resolver::iterator &it){
                    if(!code){
                        self->getSocket()->async_connect(*it, [ it,self, result,currentTime](asio::error_code code){
                            if(!code){
                                result(true,self->getSocket()->remote_endpoint());
                            } else{
                                Log::error(self->TAG,
                                           "Connect failed {}:{} ",
                                           (*it).endpoint().address().to_string(),
                                           (*it).endpoint().port());
                                result(false, tcp::endpoint());
                            }
                        });
                    } else{
                        Log::error(self->TAG,"Resolve failed {} ", code.message());
                    }
                });
    }
};


#endif //SOCKS5_SOCKETCLIENT_HPP
