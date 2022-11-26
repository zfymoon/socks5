//
// Created by curie on 2022/11/13.
//

#ifndef SOCKS5_IDATAHANDLER_HPP
#define SOCKS5_IDATAHANDLER_HPP


#include <cstddef>
#include <utility>
#include "asio.hpp"
#include "../common/Log.hpp"

using asio::ip::tcp;
using std::shared_ptr;
using std::string;
using DataCallback = std::function<void (char*)>;
using DataCallbackWithSize = std::function<void (char*, size_t,asio::error_code)>;
using ResultCallback = std::function<void (asio::error_code )>;

const int DEFAULT_BUFFER_SIZE = 1024 * 50;


class IDataHandler: public std::enable_shared_from_this<IDataHandler> {
private:
    string TAG = "IDataHandler";
    shared_ptr<tcp::socket> _socket;
    char*  _buffer;
public:
    shared_ptr<tcp::socket> getSocket() const{
        return _socket;
    }
    void read(size_t num,const DataCallback & callback){
        asio::async_read(*_socket,  asio::buffer(_buffer, num),
                         [num,callback,self = shared_from_this()](asio::error_code code, size_t length){
            if(num == length && !code){
                callback(self->_buffer);
            } else{
                Log::error(self->TAG, "Read failed {}", code.message());
            }
        });
    }
    void readSome(const DataCallbackWithSize & callback){
        _socket->async_receive(asio::buffer(_buffer, DEFAULT_BUFFER_SIZE),[=](asio::error_code code, size_t length){
            callback(_buffer, length, code);
        });
    }
    void write(const char * data, size_t num,const ResultCallback& callback){
        asio::async_write(
                *_socket,
                asio::buffer(data, num),
                [callback](asio::error_code code, size_t message_length) {
                    callback(code);
                });
    }
    IDataHandler(shared_ptr<tcp::socket> socket,
                 int bufferSize)
                    : _socket(std::move(socket)),
                     _buffer(new char[bufferSize]){

    }
    explicit IDataHandler(shared_ptr<tcp::socket> socket)
                    : IDataHandler(std::move(socket),
                                   DEFAULT_BUFFER_SIZE ){}
    virtual void closeConnection(){
        _socket->close();
    }
};


#endif //SOCKS5_IDATAHANDLER_HPP
