# WeChatFerry Python 客户端
[![PyPi](https://img.shields.io/pypi/v/wcferry.svg)](https://pypi.python.org/pypi/wcferry) [![Downloads](https://static.pepy.tech/badge/wcferry)](https://pypi.python.org/pypi/wcferry) [![Documentation Status](https://readthedocs.org/projects/wechatferry/badge/?version=latest)](https://wechatferry.readthedocs.io/zh/latest/?badge=latest)

|[📖 Python 文档](https://wechatferry.readthedocs.io/)|[📺 Python 视频教程](https://mp.weixin.qq.com/s/APdjGyZ2hllXxyG_sNCfXQ)|[🙋 FAQ](https://mp.weixin.qq.com/s/I6n_6fuQa60CrROWSgq0TA)|
|:-:|:-:|:-:|

🤖示例机器人框架：[WeChatRobot](https://github.com/lich0821/WeChatRobot)。

|![碲矿](https://raw.githubusercontent.com/lich0821/WeChatFerry/master/assets/TEQuant.jpg)|![赞赏](https://raw.githubusercontent.com/lich0821/WeChatFerry/master/assets/QR.jpeg)|
|:-:|:-:|
|后台回复 `WCF` 加群交流|如果你觉得有用|

## 快速开始
```sh
pip install --upgrade wcferry
```

### Demo：
参考 [WeChatRobot](https://github.com/lich0821/WeChatRobot) 和上面的文档。

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

### v39.2.4.0 (2024.07.08)
* 修复 wxid 问题

<details><summary>点击查看更多</summary>

版本号：`w.x.y.z`。

其中：
* `w` 是微信的大版本号，如 `37` (3.7.a.a), `38` (3.8.a.a), `39` (3.9.a.a)
* `x` 是适配的微信的小版本号，从 0 开始
* `y` 是 `WeChatFerry` 的版本，从 0 开始
* `z` 是各客户端的版本，从 0 开始

功能：

* 查询登录状态
* 获取登录账号信息
* 获取消息类型
* 获取联系人
* 获取可查询数据库
* 获取数据库所有表
* 获取语音消息
* 发送文本消息（可 @）
* 发送图片消息
* 发送文件消息
* 发送卡片消息
* 发送 GIF
* 拍一拍群友
* 转发消息
* 开启接收消息
* 关闭接收消息
* 查询数据库
* 获取朋友圈消息
* 下载图片、视频、文件
* 解密图片
* 添加群成员
* 删除群成员
* 邀请群成员

</details>
