# WeChatFerry HTTP 客户端
[![PyPi](https://img.shields.io/pypi/v/wcfhttp.svg)](https://pypi.python.org/pypi/wcfhttp)

基于 [wcferry](https://pypi.org/project/wcferry/) 封装的 HTTP 客户端。

## 快速开始
### 安装
```sh
pip install --upgrade wcfhttp
```

### 运行
```sh
# 查看版本
wcfhttp -v

# 查看帮助
wcfhttp -h

# 忽略新消息运行
wcfhttp

# 新消息转发到指定地址
wcfhttp --cb http://your_host:your_port/callback
```

### 接收消息回调接口文档
参考文档（默认地址为：http://localhost:9999/docs ）`接收消息回调样例`。
