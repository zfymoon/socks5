//
// Created by curie on 2022/11/13.
//

#ifndef SOCKS5_SOCKETCLIENT_HPP
#define SOCKS5_SOCKETCLIENT_HPP
#include <utility>

#include "SocksAddr.h"
#include "asio.hpp"
#include "memory"

using std::shared_ptr;
using asio::ip::tcp;
using std::string;
using ConnectResult = std::function<void (bool,tcp::endpoint)>;
using WriteResult = std::function<void (asio::error_code)>;
const int CLIENT_BUFFER_SIZE = 1024 * 50;
class SocketClient: public std::enable_shared_from_this<SocketClient>{
private:
    const string TAG = "SocketClient";
    shared_ptr<SocksAddr> _addr;
    shared_ptr<tcp::socket> _socket;
    tcp::resolver _resolver;
    char * _buffer;
public:
    SocketClient(shared_ptr<SocksAddr> addr, shared_ptr<tcp::socket> socket)
                : _addr(std::move(addr)),
                  _socket(std::move(socket)),_resolver(_socket->get_executor()),_buffer(new char[CLIENT_BUFFER_SIZE]){}
    void connect(const ConnectResult& result){
        _resolver.async_resolve(
                tcp::resolver::query(  _addr->getAddrName(), std::to_string(_addr->getPort())),
                [self = shared_from_this(),result](asio::error_code code, const tcp::resolver::iterator &it){
                    if(!code){
                        self->_socket->async_connect(*it, [ it,self, result](asio::error_code code){
                            if(!code){
                                result(true,self->_socket->remote_endpoint());
                            } else{
                                Log::error(self->TAG,"Connect failed {}:{} ",(*it).endpoint().address().to_string(),(*it).endpoint().port());
                                result(false, tcp::endpoint());
                            }
                        });
                    } else{
                        Log::error(self->TAG,"Resolve failed {} ", code.message());
                    }
                });
    }

    void readSome(const DataCallbackWithSize & callback){
        _socket->async_receive(asio::buffer(_buffer, CLIENT_BUFFER_SIZE),[=](asio::error_code code,size_t length){
            callback(_buffer, length, code);
        });
    }

    void writeData(char * data, size_t num,const WriteResult& result){
        asio::async_write(*_socket,asio::buffer(data,num),[=](asio::error_code code,size_t length){
            result(code);
        });
    }
    void close(){
        _socket->close();
    }
};


#endif //SOCKS5_SOCKETCLIENT_HPP
