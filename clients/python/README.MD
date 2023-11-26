# WeChatFerry Python 客户端
[![PyPi](https://img.shields.io/pypi/v/wcferry.svg)](https://pypi.python.org/pypi/wcferry) [![Downloads](https://static.pepy.tech/badge/wcferry)](https://pypi.python.org/pypi/wcferry) [![Documentation Status](https://readthedocs.org/projects/wechatferry/badge/?version=latest)](https://wechatferry.readthedocs.io/zh/latest/?badge=latest)

|[📖 文档](https://wechatferry.readthedocs.io/)|[📺 视频教程](https://mp.weixin.qq.com/s/APdjGyZ2hllXxyG_sNCfXQ)|[🙋 FAQ](https://mp.weixin.qq.com/s/vAGpn1C9stI8Xzt1hUJhLA)|
|:-:|:-:|:-:|

🤖示例机器人框架：[WeChatRobot](https://github.com/lich0821/WeChatRobot)。

## 快速开始
```sh
pip install --upgrade wcferry
```

### Demo：
参考 [WeChatRobot](https://github.com/lich0821/WeChatRobot) 和上面的文档。

|![碲矿](https://raw.githubusercontent.com/lich0821/WeChatFerry/master/assets/TEQuant.jpg)|![赞赏](https://raw.githubusercontent.com/lich0821/WeChatFerry/master/assets/QR.jpeg)|
|:-:|:-:|
|后台回复 `WeChatFerry` 加群交流|如果你觉得有用|

## 一起开发
### 配置环境
```sh
# 创建虚拟环境
python -m venv .env
# 激活虚拟环境
source .env/Scripts/activate
# 升级 pip
pip install --upgrade pip
# 安装依赖包
pip install grpcio-tools pynng
```

### 重新生成 PB 文件
```sh
# CMD
cd clients\python\wcferry
python -m grpc_tools.protoc --python_out=. --proto_path=..\..\..\WeChatFerry\rpc\proto\ wcf.proto

# GitBash
cd clients/python/wcferry
python -m grpc_tools.protoc --python_out=. --proto_path=../../../WeChatFerry/rpc/proto/ wcf.proto
```

## 版本更新
### v39.0.6 (2023.11.26)
* 修复下载图片退出问题

<details><summary>点击查看更多</summary>

版本号：`w.x.y.z`。

其中：
* `w` 是微信的大版本号，如 `37` (3.7.a.a), `38` (3.8.a.a), `39` (3.9.a.a)
* `x` 是适配的微信的小版本号，从 0 开始
* `y` 是 `WeChatFerry` 的版本，从 0 开始
* `z` 是各客户端的版本，从 0 开始

功能：

* 检查登录状态
* 获取登录账号的 wxid
* 获取消息类型
* 获取所有联系人
* 获取所有好友
* 获取数据库
* 获取某数据库下的表
* 获取用户信息
* 发送文本消息（可 @）
* 发送图片（Python 客户端支持网络路径）
* 发送文件（Python 客户端支持网络路径）
* 允许接收消息
* 停止接收消息
* 执行 SQL 查询
* 接受好友申请
* 添加群成员
* 删除群成员
* 解密图片
* 获取朋友圈消息
* 下载图片、文件、视频

</details>
