# WeChatFerry Python 客户端
⚠️ **只支持 Windows** ⚠️

🤖示例机器人框架：[WeChatRobot](https://github.com/lich0821/WeChatRobot)。

## 快速开始
```sh
pip install --upgrade wcferry
```

### 接口清单
* 检查登录状态：`is_login`
* 获取登录账号的 wxid：`get_self_wxid`
* 获取消息类型：`get_msg_types`
* 获取所有联系人：`get_contacts`
* 获取所有好友：`get_friends`
* 获取数据库：`get_dbs`
* 获取某数据库下的表：`get_tables`
* 获取用户信息：`get_user_info`
* 发送文本消息（可 @）：`send_text`
* 发送图片：`send_image`
* 发送文件：`send_file`
* 发送 XML：`send_xml`
* 发送表情：`send_emotion`
* 允许接收消息：`enable_receiving_msg` 和 `enable_recv_msg（旧接口）`
* 停止接收消息：`disable_recv_msg`
* 执行 SQL 查询：`query_sql`
* 接受好友申请：`accept_new_friend`
* 添加群成员：`add_chatroom_members`
* 解密图片：`decrypt_image`

详情查看 [client.py](wcferry/client.py)。

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
    # wcf = Wcf("tcp://127.0.0.1:10086") # 连接远端服务

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
cd python\wcferry
# CMD
python -m grpc_tools.protoc --python_out=. --proto_path=..\..\rpc\proto\ wcf.proto

# GitBash
python -m grpc_tools.protoc --python_out=. --proto_path=../../rpc/proto/ wcf.proto
```
