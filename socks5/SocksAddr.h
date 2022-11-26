//
// Created by curie on 2022/11/6.
//

#ifndef PROJECTZ_SOCKSADDR_H
#define PROJECTZ_SOCKSADDR_H

#include "vector"
#include "string"
#include "../util/NetUtil.hpp"
#include "Socks5State.hpp"
#include "../common/Log.hpp"

using std::string;
using std::vector;
class SocksAddr {
private:
    const string TAG = "Socks5Addr";
    char _addrType;
    string _addrName;
    uint16_t _addrPort;
public:
    SocksAddr(const char addrType,const char * data,size_t length){
        if(addrType == Socks5::AddrType::DOMAIN_NAME){
            _addrName = string(data, length);
        }
        _addrPort = ((data[length] & 0xFF) << 8 ) | ((data[length + 1]) & 0x00FF);
    }
    SocksAddr(const char addrType,const char * data)
                :_addrType(addrType){
        int step = 0;
        if(addrType == Socks5::AddrType::IPV4){
            step = 4;
            _addrName = asio::ip::address_v4(
                    ntohl(*((uint32_t *) &data[0]))).to_string();
        } else if(addrType == Socks5::AddrType::IPV6){
            step = 6;
            _addrName = std::string((char *)data, 6);
        }
        _addrPort = ((data[step] & 0xFF) << 8 ) | ((data[step + 1]) & 0x00FF);
    }
    [[nodiscard]] const string & getAddrName() const {
        return _addrName;
    }
    [[nodiscard]] char getAddrType() const {
        return _addrType;
    }
    [[nodiscard]] uint16_t getPort() const {
        return _addrPort;
    }
};


#endif //PROJECTZ_SOCKSADDR_H
