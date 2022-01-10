//
// Created by curie on 2022/1/7.
// manager session
//

#ifndef SOCKS5_SESSION_H
#define SOCKS5_SESSION_H
#include <memory>
#include <string>
#include <vector>
#include <array>
#include "libs/asio/include/asio.hpp"
#include <exception>
#include "log.h"

using namespace std;
using asio::ip::tcp;

/**
 *
 */
class session: public enable_shared_from_this<session> {

private:
    const string TAG = "Session";
    const static int BUFFER_SIZE = 1024 * 64;
    const static int SOURCE_BUFFER_SIZE = (BUFFER_SIZE * 16) / 64;
    const static int TARGET_BUFFER_SIZE = (BUFFER_SIZE * 48) / 64;
    const static byte SOCKS5_VERSION = byte(0x05);
    const static byte AD_TYPE_IP4 = byte(0x01);
    const static byte AD_TYPE_DOMAIN = byte(0x03);
    const static byte AD_TYPE_IP6 = byte(0x04);
    string target_name;
    string target_port;
    array<byte, SOURCE_BUFFER_SIZE> _buffer;
    array<byte, TARGET_BUFFER_SIZE> _target_buffer;
    tcp::socket _socket;
    tcp::socket _target_socket;
    tcp::resolver _resolver;
    int _session_id;
    bool _closed = false;

public:
    session(tcp::socket&& socket, int session_id): _socket(move(socket)),
                                                   _session_id(session_id),
                                                   _buffer(),
                                                   _target_buffer(),
                                                   _resolver(_socket.get_executor()),
                                                   _target_socket(_socket.get_executor()){

    }

    void start(){
        auto self(shared_from_this());
        log::d(TAG, "[start] session:" + to_string(_session_id));
        asio::async_read(_socket,
                         asio::buffer(_buffer, 2),
                         [this, self](asio::error_code code, size_t message_length){
            if(!code){
                if(message_length == 2){
                    byte version = _buffer[0];
                    int num_method = to_integer<int>(_buffer[1]);
                    if(version == SOCKS5_VERSION){
                        try{
                        size_t read_method_length = asio::read(_socket, asio::buffer(_buffer, num_method));
                        if(read_method_length == num_method){
                            _buffer[0] = SOCKS5_VERSION;
                            //no verify
                            _buffer[1] = byte(0x00);
                                size_t write_length = asio::write(_socket,
                                                                  asio::buffer(_buffer, 2));
                                if(write_length == 2){
                                    size_t r_size = asio::read(_socket, asio::buffer(_buffer, 4));
                                    if(r_size == 4){
                                        byte v = _buffer[0];
                                        byte cmd = _buffer[1];
                                        byte ad_type = _buffer[3];
                                        if(v == SOCKS5_VERSION){
                                            size_t ad_size = 0;
                                            size_t read_ad_size;
                                            switch (ad_type) {
                                                case AD_TYPE_IP4:
                                                    ad_size = 4;
                                                    break;
                                                case AD_TYPE_IP6:
                                                    ad_size = 16;
                                                    break;
                                                case AD_TYPE_DOMAIN:
                                                    read_ad_size = asio::read(_socket,asio::buffer(_buffer, 1));
                                                    if(read_ad_size == 1){
                                                        ad_size = to_integer<int>(_buffer[0]);
                                                    } else{
                                                        log::e(TAG,"invalid domain addr");
                                                    }
                                                    break;
                                                default:
                                                    log::e(TAG,"invalid ad type");
                                                    break;
                                            }
                                            size_t ad_total_size = ad_size + 2;
                                            read_ad_size = asio::read(_socket,asio::buffer(_buffer, ad_total_size));
                                            if(read_ad_size == ad_total_size){
                                                if(ad_type == AD_TYPE_IP4) {
                                                    target_name = move(asio::ip::address_v4(
                                                            ntohl(*((uint32_t *) &_buffer[0]))).to_string());
                                                    target_port = move(std::to_string(ntohs(*((uint16_t *) &_buffer[4]))));
                                                } else if(ad_type == AD_TYPE_DOMAIN){
                                                    target_name = std::string((char *)_buffer.data(), ad_size);
                                                    target_port = std::to_string(ntohs(*((uint16_t*)&_buffer[0 + ad_size])));
                                                }
                                                if(target_name.empty() || target_port.empty()){
                                                    log::e(TAG,"empty addr");
                                                    close();
                                                } else{
                                                    _resolver.async_resolve(
                                                            tcp::resolver ::query({target_name, target_port}),
                                                            [this, self](asio::error_code code, const tcp::resolver::iterator &it){
                                                                _target_socket.async_connect(
                                                                        *it,
                                                                        [this,self](asio::error_code code){
                                                                            if(!code){
                                                                                _buffer[0] = SOCKS5_VERSION;
                                                                                _buffer[1] = byte(0x00);
                                                                                _buffer[2] = byte(0x00);
                                                                                auto target_addr = _target_socket.remote_endpoint().address();
                                                                                if(target_addr.is_v4()){
                                                                                    _buffer[3] =  AD_TYPE_IP4;
                                                                                    auto real_target_ip = target_addr.to_v4().to_ulong();
                                                                                    auto real_target_port = htons(_target_socket.remote_endpoint().port());
                                                                                    memcpy(_buffer.data() + 4, &real_target_ip, 4);
                                                                                    memcpy(_buffer.data() + 8, &real_target_port, 2);
                                                                                } else{
                                                                                    //ipv6
                                                                                }
                                                                                size_t connect_rep_length = asio::write(_socket, asio::buffer(_buffer, 10));
                                                                                if(connect_rep_length == 10){
                                                                                    swap_data();
                                                                                }
                                                                            } else{
                                                                                close();
                                                                                log::d(TAG, "connect target failed");
                                                                            }
                                                                        });
                                                            });
                                                }
                                            } else{
                                                log::e(TAG,"invalid ad length ");
                                            }
                                        }
                                    }
                                }else{

                                }
                        }
                        }catch (exception &e){
                            log::e(TAG,"[start] err "+ string(e.what()));
                        }
                    }
                }
            } else{
                log::e(TAG,"[start] err "+ to_string(code.value()));
            }

        });
    }

    void close(){
        try{
            if(!_closed){
                _closed = true;
                _socket.close();
                _target_socket.close();
                log::d(TAG,"[close] session:"+ to_string(_session_id));
            }
        }catch (exception &e){
            log::e(TAG,"[close] "+ string(e.what()));
        }
    }
    /**
     * swap dat:
     *  _socket
     *  _target_socket
     */
    void swap_data(){
        read_source_data();
        read_target_data();
    }

    void read_source_data(){
        if(_closed){
            return;
        }
        auto self(shared_from_this());
        _socket.async_receive(asio::buffer(_buffer,SOURCE_BUFFER_SIZE),
                              [this, self](asio::error_code code, size_t message_length){
            if(!code && !_closed){
                write_to_target(message_length);
            } else{
                if(code == asio::error::eof){
                    close();
                    return;
                }
            }
        });
    }
    void read_target_data(){
        if(_closed){
            return;
        }
        auto self(shared_from_this());
        _target_socket.async_receive(
                asio::buffer(_target_buffer,TARGET_BUFFER_SIZE),
                [this, self](asio::error_code code, size_t message_length){
                    if(!code && !_closed){
                        write_to_source(message_length);
                    } else{
                        if(code == asio::error::eof){
                            close();
                            return;
                        }
                    }
                });
    }
    void write_to_target(size_t data_size){
        if(_closed){
            return;
        }
        auto self(shared_from_this());
        asio::async_write(
                _target_socket,
                asio::buffer(_buffer, data_size),
                [this, self](asio::error_code code, size_t message_length){
                                if(!code && !_closed){
                                    read_source_data();
                                } else{
                                    close();
                                    return;                                }
                          });

    }
    void write_to_source(size_t data_size){
        if(_closed){
            return;
        }
        auto self(shared_from_this());
        asio::async_write(
                _socket,
                asio::buffer(_target_buffer, data_size),
                [this, self](asio::error_code code, size_t message_length){
                              if(!code && !_closed){
                                  read_target_data();
                              } else{
                                  close();
                                  return;
                              }
                          });
    }
};

#endif //SOCKS5_SESSION_H
