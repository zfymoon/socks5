//
// Created by curie on 2022/11/5.
//

#ifndef PROJECTZ_SOCKS5STATE_H
#define PROJECTZ_SOCKS5STATE_H


namespace Socks5{
    enum State {
        INITED,
        METHOD_CHOOSE,
        AUTH,
        EXEC_COMMAND,
        READY,
        DESTROYED
    };
    const char Version = 0x05;
    namespace Method {
        const char NO_AUTH = 0x00;
        const char GSSAPI  = 0x01;
        const char PASSWD  = 0x02;
        const char NONE    = 0xFF;
    }
    namespace AuthState {
        const char SUCC    = 0x00;
    }
    namespace Command {
        const char CONNECT = 0x01;
        const char BIND    = 0x02;
        const char UDP     = 0x03;
    }
    namespace AddrType {
        const char IPV4    = 0x01;
        const char DOMAIN_NAME  = 0x03;
        const char IPV6    = 0x04;
    }
    namespace ResponseState{
        const char SUCCESS = 0x00;
        const char FAILED  = 0x01;
    }
}
#endif //PROJECTZ_SOCKS5STATE_H
