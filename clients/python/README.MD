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
```py
#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
from threading import Thread
from time import sleep

from wcferry import Wcf

logging.basicConfig(level='DEBUG', format="%(asctime)s %(message)s")
LOG = logging.getLogger("Demo")


def process_msg(wcf: Wcf):
    """处理接收到的消息"""
    while wcf.is_receiving_msg():
        try:
            msg = wcf.get_msg()
        except Exception as e:
            continue

        LOG.info(msg)  # 简单打印


def main():
    LOG.info("Start demo...")
    wcf = Wcf(debug=True)             # 默认连接本地服务

    sleep(5)  # 等微信加载好，以免信息显示异常
    LOG.info(f"已经登录: {True if wcf.is_login() else False}")
    LOG.info(f"wxid: {wcf.get_self_wxid()}")

    # 允许接收消息
    # wcf.enable_recv_msg(LOG.info) # deprecated

    # 允许接收消息
    wcf.enable_receiving_msg(pyq=True)  # 同时允许接收朋友圈消息
    Thread(target=process_msg, name="GetMessage", args=(wcf,), daemon=True).start()

    # wcf.disable_recv_msg() # 当需要停止接收消息时调用
    sleep(5)
    ret = wcf.send_text("Hello world.", "filehelper")
    LOG.info(f"send_text: {ret}")

    sleep(5)
    # 需要确保图片路径正确，建议使用绝对路径（使用双斜杠\\）
    ret = wcf.send_image("https://raw.githubusercontent.com/lich0821/WeChatFerry/master/assets/QR.jpeg", "filehelper")
    LOG.info(f"send_image: {ret}")

    sleep(5)
    # 需要确保文件路径正确，建议使用绝对路径（使用双斜杠\\）
    ret = wcf.send_file("https://raw.githubusercontent.com/lich0821/WeChatFerry/master/README.MD", "filehelper")
    LOG.info(f"send_file: {ret}")

    sleep(5)
    LOG.info(f"Message types:\n{wcf.get_msg_types()}")
    LOG.info(f"Contacts:\n{wcf.get_contacts()}")

    sleep(5)
    LOG.info(f"DBs:\n{wcf.get_dbs()}")
    LOG.info(f"Tables:\n{wcf.get_tables('db')}")
    LOG.info(f"Results:\n{wcf.query_sql('MicroMsg.db', 'SELECT * FROM Contact LIMIT 1;')}")

    # 需要真正的 V3、V4 信息
    # wcf.accept_new_friend("v3", "v4")

    # 添加群成员，填写正确的群 ID 和成员 wxid
    # ret = wcf.add_chatroom_members("chatroom id", "wxid1,wxid2,wxid3,...")
    # LOG.info(f"add_chatroom_members: {ret}")

    # 删除群成员，填写正确的群 ID 和成员 wxid
    # ret = wcf.del_chatroom_members("chatroom id", "wxid1,wxid2,wxid3,...")
    # LOG.info(f"add_chatroom_members: {ret}")

    sleep(5)
    wcf.refresh_pyq(0)  # 刷新朋友圈第一页
    # wcf.refresh_pyq(id)  # 从 id 开始刷新朋友圈

    # 一直运行
    wcf.keep_running()


if __name__ == "__main__":
    main()

```

|![碲矿](https://gitee.com/lch0821/WeChatFerry/raw/master/assets/TEQuant.jpg)|![赞赏](https://gitee.com/lch0821/WeChatFerry/raw/master/assets/QR.jpeg)|
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
### 39.0.2.1 (2023.08.01)
* 发送文件支持网络路径

<details><summary>点击查看更多</summary>

### 39.0.2.0 (2023.07.16)
* 获取朋友圈消息

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
* 某功能（Breaking Change）

</details>
