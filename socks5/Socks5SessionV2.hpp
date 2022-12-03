//
// Created by curie on 2022/11/13.
//

#ifndef SOCKS5_SOCKS5SESSIONV2_HPP
#define SOCKS5_SOCKS5SESSIONV2_HPP
#include <utility>

#include "DataHandler.hpp"
#include "asio.hpp"
#include "memory"
#include "Socks5State.hpp"
#include "SocksAddr.h"
#include "client/SocketClient.hpp"
#include "./config/Config.hpp"

using std::shared_ptr;
using asio::ip::tcp;
using Socks5::State;
using std::string;


class Socks5SessionV2 : public DataHandler {
public:
    Socks5SessionV2(shared_ptr<tcp::socket> socket,int sessionID,shared_ptr<Config> config)
            : DataHandler(std::move(socket)), _state(State::INITED), _sessionID(sessionID), _config(std::move(config)) {
        Socks5SessionV2::TAG += std::to_string(sessionID);
    }

    virtual void start(){
        setState(State::INITED);
    }
    virtual void close(const string &message){
        if(isReady()) {
            setState(State::DESTROYED);
            _targetClient->closeConnection();
            self<Socks5SessionV2>()->closeConnection();
        }
    }
    bool  isReady() const override{
        return _state == State::READY;
    }
private:
    string TAG = "Socks5SessionV2";
    int _sessionID;
    shared_ptr<Config> _config;
    char _currentMethod {};
    State _state;
    shared_ptr<SocksAddr> _addr;
    shared_ptr<SocketClient> _targetClient;
    char _currentCommand {};
    bool checkVersion(const char* data){
        if(data[0] == Socks5::Version){
            return true;
        } else{
            Log::error(TAG,"Version not match {}",data[0]);
            close("Version not match");
            return false;
        }
    }
    void notifyCommandUpdate(){
        switch (_state) {
            case State::INITED:{
                onInit();
                break;
            }
            case State::METHOD_CHOOSE:{
                onMethodChoose();
                break;
            }
            case State::AUTH:{
                onAuth();
                break;
            }
            case State::EXEC_COMMAND:{
                onCommand();
                break;
            }
            case State::READY:{
                notifyDataUpdate();
                break;
            }
            default:{}
        }
    }



protected:

    void setState(State state){
        _state = state;
        notifyCommandUpdate();
    }
    virtual void notifyDataUpdate(){
        bind(_targetClient);
    }
    virtual char getMethod(char * data, int methodNum){
        if(_config->auth){
            _currentMethod = Socks5::Method::PASSWD;
        } else{
            _currentMethod = Socks5::Method::NO_AUTH;
        }
        return _currentMethod;
    }
    virtual bool onAuth(const string &userName,const string &password){
        auto user = _config->getUser(userName);
        Log::debug(TAG,"onAuth {} {}",userName,password);
        if(user.has_value() && password.length() >= USER_TOKEN_MIN){
            auto info = *user;
            return password == info.token;
        } else{
            return false;
        }
    }
    virtual void onMethodChoose(){
        if(_currentMethod == Socks5::Method::PASSWD){
            read( 2, [self = self<Socks5SessionV2>()](const char* data){
                int nameLength = data[1];
                self->read(nameLength + 1, [self,nameLength](char * data){
                    int passwordLength = data[nameLength];
                    string name(data,nameLength);
                    self->read(passwordLength,[self,passwordLength,name](char *data){
                        string password(data,passwordLength);
                        if(self->onAuth(name,password)){
                            char resp[] = { Socks5::Version,Socks5::AuthState::SUCC};
                            self->write(resp,2, [self](asio::error_code result){
                                if(!result) {
                                    self->setState(State::AUTH);
                                } else{
                                    self->close("Auth response error");
                                }
                            });
                        } else{
                            self->close("Invalid user");
                        }
                    });
                });
            });
        } else if(_currentMethod == Socks5::Method::NO_AUTH){
            setState(State::AUTH);
        }
    }

    /**
     * 从client读取数据
     */
    virtual void onAuth(){
        read(4, [self = self<Socks5SessionV2>()](const char* data){
            if(self->checkVersion(data)){
                char addrType = data[3];
                self->_currentCommand = data[1];
                int readNum = 2;
                switch (addrType){
                    case Socks5::AddrType::IPV6:
                        readNum += 2;
                    case Socks5::AddrType::IPV4:
                        readNum += 4;
                        self->read(readNum, [self,addrType](const char * data){
                            self->_addr = std::make_shared<SocksAddr>(addrType, data);
                            self->setState(State::EXEC_COMMAND);
                        });
                        break;
                    case Socks5::AddrType::DOMAIN_NAME:{
                        self->read(1, [self,addrType](const char * data){
                            int readNum = data[0];
                            self->read(readNum, [=](const char * data){
                                self->_addr = std::make_shared<SocksAddr>(addrType, data, readNum);
                                self->setState(State::EXEC_COMMAND);
                            });
                        });
                        break;
                    }
                    default:
                        Log::error(self->TAG,"Invalid addrType {}", std::to_string(addrType));
                }
            }
        });
    }
    void onCommand(){
        switch (_currentCommand) {
            case Socks5::Command::CONNECT:
                try{
                    _targetClient = std::make_shared<SocketClient>(_addr,std::make_shared<tcp::socket>(getSocket()->get_executor()));
                }catch (std::exception &e){
                    Log::error(TAG,"error {}",e.what());
                }
                _targetClient->connect([self = self<Socks5SessionV2>()](bool result,const tcp::endpoint& endpoint){
                    if(result) {
                        char totalCommand[10];
                        totalCommand[0] = Socks5::Version;
                        totalCommand[1] = 0x00;
                        totalCommand[2] = 0x00;
                        auto targetAddr = endpoint.address();
                        if (targetAddr.is_v4()) {
                            totalCommand[3] = Socks5::AddrType::IPV4;
                            auto real_target_ip = targetAddr.to_v4().to_ulong();
                            auto real_target_port = htons(endpoint.port());
                            memcpy(totalCommand + 4, &real_target_ip, 4);
                            memcpy(totalCommand + 8, &real_target_port, 2);
                        } else {
                            //ipv6
                        }
                        self->write(totalCommand, sizeof(totalCommand), [self](asio::error_code result) {
                            if (!result) {
                                self->setState(State::READY);
                            } else {
                                self->close("Connect remote Write failed ");
                            }
                        });
                    } else{
                        self->close("Connect failed.");
                    }
                });
                break;
            default:
                break;
        }
    }
    void onBindDataError(asio::error_code code) override{
        close(code.message());
    }
    virtual void onInit(){
        read( 2, [self = self<Socks5SessionV2>()](const char* data){
            if(self->checkVersion(data)){
                int methodNum = data[1];
                self->read(methodNum, [methodNum,self](char * data){
                    char method = self->getMethod(data, methodNum);
                    char methodChoose[]  = {Socks5::Version, method};
                    self->write(methodChoose, 2 , [self](asio::error_code result){
                        if(!result){
                            self->setState(State::METHOD_CHOOSE);
                        }else{
                            Log::error(self->TAG,"Write failed");
                        }
                    });
                });
            }
        });
    }
};


#endif //SOCKS5_SOCKS5SESSIONV2_HPP
