//
// Created by curie on 2022/11/29.
//

#ifndef SOCKS5_FORWARDSESSION_HPP
#define SOCKS5_FORWARDSESSION_HPP

#include <utility>

#include "memory"
#include "../client/SocketClient.hpp"
#include "../Socks5SessionV2.hpp"
#include "../config/Config.hpp"
#include "../Socks5State.hpp"
#include "Socks5Client.hpp"
#include "string"
#include "asio.hpp"

using std::shared_ptr;
using asio::ip::tcp;
using super = Socks5SessionV2;

class ForwardSession: public Socks5SessionV2 {
private:
    shared_ptr<Socks5Client> _socks5Client;
    bool _clientReady{};
    bool _targetReady{};
public:
    const string TAG = "ForwardSession";
    ForwardSession(shared_ptr<tcp::socket> socket,
                   int sessionID,
                   shared_ptr<Config> config)
    : Socks5SessionV2(socket,sessionID,config),
      _socks5Client(
              std::make_shared<Socks5Client>(
                      config->forwardServer.server,
                      config->forwardServer.port,
                      std::make_shared<tcp::socket>(socket->get_executor()),
                      config->forwardServer.user.name,
                      config->forwardServer.user.token
                      )) {

    }
    void start() override{
        _socks5Client->connect([self = self<ForwardSession>()](bool result,const tcp::endpoint& endpoint){
            if(result){
                self->_socks5Client->auth([self](bool  result){
                    if(result){
                        self->_targetReady = true;
                        self->notifyReady();
                    }
                });
            }
        });
        super::start();
    }

    void onInit() override{
        super::onInit();
    }
    void onMethodChoose() override{
        super::onMethodChoose();
    }
    void onAuth() override{
        _clientReady = true;
        notifyReady();
    }
    void notifyReady(){
        if(_clientReady && _targetReady) {
            setState(Socks5::State::READY);
        }
    }
    void notifyDataUpdate() override {
        bind(_socks5Client);
    }
    /**
     * 转发默认不校验
     * @param data
     * @param methodNum
     * @return
     */
    char getMethod(char *data, int methodNum) override{
        return Socks5::Method::NO_AUTH;
    }
    bool onAuth(const std::string &userName, const std::string &password) override{
        return true;
    }
    ~ForwardSession(){
        Log::debug(TAG,"destroy");
    }

    void close(const std::string &message) override{
        if(super::isReady()){
            setState(Socks5::State::DESTROYED);
            closeConnection();
            _socks5Client->close(message);
        }
    }
};


#endif //SOCKS5_FORWARDSESSION_HPP
