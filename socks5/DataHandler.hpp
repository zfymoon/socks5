//
// Created by curie on 2022/11/13.
//

#ifndef SOCKS5_DATAHANDLER_HPP
#define SOCKS5_DATAHANDLER_HPP


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
/**
 * 默认缓存 50KB
 */
const int DEFAULT_BUFFER_SIZE = 1024 * 50;
/**
 * 封装数据的读写以及交换
 * 交换参考bind方法
 */
class DataHandler: public std::enable_shared_from_this<DataHandler> {
private:
    string TAG = "DataHandler";
    shared_ptr<tcp::socket> _socket;
    shared_ptr<DataHandler> _targetDataHandler{};
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
                Log::error(self->TAG, "Read failed {}",code.message());
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
    template<class T>
    shared_ptr<T> self(){
        return std::dynamic_pointer_cast<T>(shared_from_this());
    }
    DataHandler(shared_ptr<tcp::socket> socket,
                int bufferSize)
                    : _socket(std::move(socket)),
                     _buffer(new char[bufferSize]){

    }
    explicit DataHandler(shared_ptr<tcp::socket> socket)
                    : DataHandler(std::move(socket),
                                  DEFAULT_BUFFER_SIZE ){}
    /**
     *
     * @param target
     */
    void bind(shared_ptr<DataHandler> target){
        _targetDataHandler = std::move(target);
        sourceToTarget();
        targetToSource();
    }
    void sourceToTarget(){
        if(isReady()) {
            readSome([self = self<DataHandler>()](char *data, size_t size, asio::error_code code) {
                if (!code) {
                    self->_targetDataHandler->write(data, size, [self](asio::error_code code) {

                        if (!code) {
                            self->sourceToTarget();
                        } else {
                            self->onBindDataError(code);
                        }
                    });
                } else {
                    self->onBindDataError(code);
                }
            });
        }
    }
    void targetToSource(){
        if(isReady()) {
            _targetDataHandler->readSome([self = self<DataHandler>()](char *data, size_t size, asio::error_code code) {
                if (!code) {
                    self->write(data, size, [self](asio::error_code code) {
                        if (!code) {
                            self->targetToSource();
                        } else {
                            self->onBindDataError(code);
                        }
                    });
                } else {
                    self->onBindDataError(code);
                }
            });
        }
    }
    virtual void onBindDataError(asio::error_code code){
    }
    virtual bool isReady() const {
        return true;
    }
    virtual void closeConnection(){
        _socket->close();
        delete []_buffer;
    }
};


#endif //SOCKS5_DATAHANDLER_HPP
