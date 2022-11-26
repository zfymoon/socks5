#include "socks5/Socks5Server.hpp"
#include "common/Log.hpp"
#include "common/FileUtil.hpp"
#include "iostream"
int main() {
    Log::init();
    std::cout<<FileUtil::readContent("")<<endl;
    std::shared_ptr<Socks5Server> socks5Server = make_shared<Socks5Server>(9983);
    socks5Server->run();
    return 0;
}
