# WeChatFerry HTTP 客户端
[![PyPi](https://img.shields.io/pypi/v/wcfhttp.svg)](https://pypi.python.org/pypi/wcfhttp) [![Downloads](https://static.pepy.tech/badge/wcfhttp)](https://pypi.python.org/pypi/wcfhttp) [![Documentation Status](https://readthedocs.org/projects/wechatferry/badge/?version=latest)](https://wechatferry.readthedocs.io/zh/latest/?badge=latest)

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

|![碲矿](https://raw.githubusercontent.com/lich0821/WeChatFerry/master/TEQuant.jpg)|![赞赏](https://raw.githubusercontent.com/lich0821/WeChatFerry/master/QR.jpeg)|
|:-:|:-:|
|后台回复 `WeChatFerry` 加群交流|如果你觉得有用|
