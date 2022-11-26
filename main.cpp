#include "socks5/Socks5Server.hpp"
#include "common/Log.hpp"
#include "iostream"
int main() {
    Log::init();
    auto socks5Server = make_shared<Socks5Server>("../config.json");
    socks5Server->run();
    return 0;
}
