# 目标
使用c++和asio实现的socks5代理，并且解决了部分客户端软件(例如chrome浏览器)不方便设置代理身份验证的问题。
# 使用
## 直接使用
添加好配置文件直接运行即可
## 本地服务中转
forward参数需要好好配置，其中的服务器地址和端口是一个远端的正常代理服务器。
然后正常运行即可
# 配置文件
```
{
  "port": xxx,  #本地端口
  "auth": true, #是否开启用户名校验
  "user": [     #允许用用户列表
    {
      "name": "name",
      "token": "tokenxxx"
    }
  ],
  "forward": {  #转发服务器
    "enable": true,
    "host": "xxx.xxx.xxx.xxx",
    "port": xxx,
    "user": {
        "name": "name",
        "token": "tokenxxx"
      }
  }
}
```
## 测试
```
curl --socks5 localhost:9981 https://www.baidu.com/
```

