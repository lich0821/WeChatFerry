# WeChatFerry Python 客户端
[![PyPi](https://img.shields.io/pypi/v/wcferry.svg)](https://pypi.python.org/pypi/wcferry) [![Downloads](https://static.pepy.tech/badge/wcferry)](https://pypi.python.org/pypi/wcferry) [![Documentation Status](https://readthedocs.org/projects/wechatferry/badge/?version=latest)](https://wechatferry.readthedocs.io/zh/latest/?badge=latest)

|[📖 文档](https://wechatferry.readthedocs.io/)|[📺 视频教程](https://mp.weixin.qq.com/s/APdjGyZ2hllXxyG_sNCfXQ)|[🙋 FAQ](https://mp.weixin.qq.com/s/XTJ9H-FsCPCscixAts8i_A)|
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
    wcf.enable_receiving_msg()
    Thread(target=process_msg, name="GetMessage", args=(wcf,), daemon=True).start()

    # wcf.disable_recv_msg() # 当需要停止接收消息时调用

    ret = wcf.send_text("Hello world.", "filehelper")
    LOG.info(f"send_text: {ret}")

    ret = wcf.send_image("TEQuant.jpeg", "filehelper")  # 需要确保图片路径正确
    LOG.info(f"send_image: {ret}")

    ret = wcf.send_file("README.MD", "filehelper")  # 需要确保文件路径正确
    LOG.info(f"send_file: {ret}")

    LOG.info(f"Message types:\n{wcf.get_msg_types()}")
    LOG.info(f"Contacts:\n{wcf.get_contacts()}")

    LOG.info(f"DBs:\n{wcf.get_dbs()}")
    LOG.info(f"Tables:\n{wcf.get_tables('db')}")
    LOG.info(f"Results:\n{wcf.query_sql('MicroMsg.db', 'SELECT * FROM Contact LIMIT 1;')}")

    # 需要真正的 V3、V4 信息
    # wcf.accept_new_friend("v3", "v4")

    # 填写正确的群 ID 和成员 wxid
    # ret = wcf.add_chatroom_members("chatroom id", "wxid1,wxid2,wxid3,...")
    # LOG.info(f"add_chatroom_members: {ret}")

    xml = '<?xml version="1.0"?><msg><appmsg appid="" sdkver="0"><title>叮当药房，24小时服务，28分钟送药到家！</title><des>叮当快药首家承诺范围内28分钟送药到家！叮当快药核心区域内7*24小时全天候服务，送药上门！叮当快药官网为您提供快捷便利，正品低价，安全放心的购药、送药服务体验。</des><action>view</action><type>33</type><showtype>0</showtype><content /><url>https://mp.weixin.qq.com/mp/waerrpage?appid=wxc2edadc87077fa2a&amp;type=upgrade&amp;upgradetype=3#wechat_redirect</url><dataurl /><lowurl /><lowdataurl /><recorditem /><thumburl /><messageaction /><md5>7f6f49d301ebf47100199b8a4fcf4de4</md5><extinfo /><sourceusername>gh_c2b88a38c424@app</sourceusername><sourcedisplayname>叮当快药 药店送药到家夜间买药</sourcedisplayname><commenturl /><appattach><totallen>0</totallen><attachid /><emoticonmd5></emoticonmd5><fileext>jpg</fileext><filekey>da0e08f5c7259d03da150d5e7ca6d950</filekey><cdnthumburl>3057020100044b30490201000204e4c0232702032f4ef20204a6bace6f02046401f62d042430326337303430352d333734332d343362652d623335322d6233333566623266376334620204012400030201000405004c537600</cdnthumburl><aeskey>0db26456caf243fbd4efb99058a01d66</aeskey><cdnthumbaeskey>0db26456caf243fbd4efb99058a01d66</cdnthumbaeskey><encryver>1</encryver><cdnthumblength>61558</cdnthumblength><cdnthumbheight>100</cdnthumbheight><cdnthumbwidth>100</cdnthumbwidth></appattach><weappinfo><pagepath>pages/index/index.html</pagepath><username>gh_c2b88a38c424@app</username><appid>wxc2edadc87077fa2a</appid><version>197</version><type>2</type><weappiconurl>http://wx.qlogo.cn/mmhead/Q3auHgzwzM4727n0NQ0ZIPQPlfp15m1WLsnrXbo1kLhFGcolgLyc0A/96</weappiconurl><appservicetype>0</appservicetype><shareId>1_wxc2edadc87077fa2a_29177e9a9b918cb9e75964f80bb8f32e_1677849476_0</shareId></weappinfo><websearch /></appmsg><fromusername>wxid_xxxxxxxxxxxxxx</fromusername><scene>0</scene><appinfo><version>1</version><appname /></appinfo><commenturl /></msg>'
    ret = wcf.send_xml("filehelper", xml, 0x21)
    LOG.info(f"send_xml: {ret}")

    ret = wcf.send_emotion("emo.gif", "filehelper")  # 需要确保 gif 路径正确
    LOG.info(f"send_emotion: {ret}")

    # 一直运行
    wcf.keep_running()


if __name__ == "__main__":
    main()

```

|![碲矿](https://raw.githubusercontent.com/lich0821/WeChatFerry/master/TEQuant.jpg)|![赞赏](https://raw.githubusercontent.com/lich0821/WeChatFerry/master/QR.jpeg)|
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
cd python\wcferry
python -m grpc_tools.protoc --python_out=. --proto_path=..\..\rpc\proto\ wcf.proto

# GitBash
cd python/wcferry
python -m grpc_tools.protoc --python_out=. --proto_path=../../rpc/proto/ wcf.proto
```

## 版本更新
版本号：`w.x.y.z`。

其中：
* `w` 是微信的大版本号，如 `37` (3.7.a.a), `38` (3.8.a.a), `39` (3.9.a.a)
* `x` 是适配的微信的小版本号，从 0 开始
* `y` 是 `WeChatFerry` 的版本，从 0 开始
* `z` 是各客户端的版本，从 0 开始

### 37.1.25.5 (2023.05.19)
支持 `3.7.0.30` 的最后一个版本。

功能：

* 检查登录状态
* 获取登录账号的 wxid
* 获取消息类型
* 获取所有联系人
* 获取所有好友
* 获取数据库
* 获取某数据库下的表
* 发送文本消息（可 @）
* 发送图片（Python 客户端支持网络路径）
* 发送文件
* 发送 XML
* 发送表情
* 允许接收消息
* 停止接收消息
* 执行 SQL 查询
* 接受好友申请
* 添加群成员
* 解密图片
* 某功能

<details><summary>历史更新</summary>

</details>
