//
// Created by curie on 2022/12/3.
//

#ifndef SOCKS5_SOCKS5CLIENT_HPP
#define SOCKS5_SOCKS5CLIENT_HPP
#include <utility>

#include "memory"
#include "string"
#include "../client/SocketClient.hpp"
#include "asio.hpp"
using std::string;
using asio::ip::tcp;
using std::shared_ptr;

using AuthCallback = std::function<void (bool)>;
/**
 * Socks5 客户端，默认只支持用户名密码校验
 */
class Socks5Client: public SocketClient{

private:
    string TAG = "Socks5Client";
    Socks5::State _state {};
    string _userName;
    string _password;
    AuthCallback _callback{};
    bool checkVersion(const char* data){
        if(data[0] == Socks5::Version){
            return true;
        } else{
            Log::error(TAG,"Version not match {}",data[0]);
            close("Version not match");
            return false;
        }
    }
public:
    Socks5Client(const string& server,
                 int port ,
                 shared_ptr<tcp::socket> socket,
                 string  name,
                 string token)
        : SocketClient(server,port,std::move(socket)),
            _state(Socks5::State::INITED),
            _userName(std::move(name)),
            _password(std::move(token)){
    }
    void auth(AuthCallback callback){
        _callback = callback;
        setState(State::INITED);
    }
    void notifyCommandUpdate(){
        switch (_state) {
            case Socks5::State::INITED:
                onInit();
                break;
            case Socks5::State::METHOD_CHOOSE:
                onMethodChoose();
                break;
            case Socks5::State::AUTH:
                onAuth();
                break;
            default:{};
        }

    }
    bool isReady() const {
        return _state == Socks5::State::READY;
    }
    void onAuth(){
        read(2,[ self = self<Socks5Client>()](char *data){
            if(self->checkVersion(data)){
                self->setState(Socks5::State::READY);
                self->_callback(true);
            }
        });
    }
    void onMethodChoose(){
        read(2,[ self = self<Socks5Client>()](char *data){
            if(self->checkVersion(data)){
                char method = data[1];
                if(method == self->getMethod()){
                    //write name and password
                    auto userInfo = self->getUserInfo();
                    char authRequest [3 + userInfo.first.size() + userInfo.second.size()];
                    char nameLength = userInfo.first.length();
                    char tokenLength = userInfo.second.length();
                    authRequest[0] = Socks5::Version;
                    authRequest[1] = nameLength;
                    authRequest[nameLength + 2] = tokenLength;
                    memcpy(authRequest + 2,userInfo.first.c_str(),nameLength );
                    memcpy(authRequest + 3 + nameLength, userInfo.second.c_str(),tokenLength);
                    self->write(authRequest,sizeof(authRequest),[self](asio::error_code code){
                        if(!code){
                            self->setState(Socks5::State::AUTH);
                        } else{
                            self->close("Write failed");
                        }
                    });
                } else{
                    self->close("Method not match");
                }
            }
        });
    }
    void setState(State state){
        _state = state;
        notifyCommandUpdate();
    }
    void onInit(){
        char  data[] = { Socks5::Version,1,getMethod()};
        write(data, sizeof(data),[ self = self<Socks5Client>()](asio::error_code code){
            if(!code){
                self->setState(Socks5::State::METHOD_CHOOSE);
            } else{
                self->close(code.message());
            }
        });
    }
    pair<string,string> getUserInfo(){
        return {_userName,_password};
    }
    void close(const string & message){
        closeConnection();
    }
    char getMethod(){
        return Socks5::Method::PASSWD;
    }

};


#endif //SOCKS5_SOCKS5CLIENT_HPP
