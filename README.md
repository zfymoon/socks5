# socks5
使用c++和asio 实现的socks5代理
# 使用
```#include "server.h"
int main() {
    server proxy_server(9981);
    proxy_server.run();
    return 0;
}
```
# 测试
## curl
```
curl --socks5 localhost:9981 https://www.baidu.com/
```
## browser
```
https://support.mozilla.org/en-US/kb/connection-settings-firefox
```
# 计划
#### 实现身份验证
#### 实现ipv6

