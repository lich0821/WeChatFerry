# WeChatFerry Python 客户端
⚠️ **只支持 Windows** ⚠️

## 快速开始
```sh
pip install wcferry
```

参考 Demo：
```py
#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
from threading import Thread

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

    ret = wcf.send_image("TEQuant.jpeg", "filehelper")
    LOG.info(f"send_image: {ret}")

    LOG.info(f"Message types:\n{wcf.get_msg_types()}")
    LOG.info(f"Contacts:\n{wcf.get_contacts()}")

    LOG.info(f"DBs:\n{wcf.get_dbs()}")
    LOG.info(f"Tables:\n{wcf.get_tables('db')}")
    LOG.info(f"Results:\n{wcf.query_sql('MicroMsg.db', 'SELECT * FROM Contact LIMIT 1;')}")

    # wcf.accept_new_friend("v3", "v4") # 需要真正的 V3、V4 信息

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
python -m grpc_tools.protoc --python_out=. --proto_path=..\..\rpc\proto\ wcf.proto
```

### 参考项目 [README](../README.MD)
