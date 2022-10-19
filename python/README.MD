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

import signal
from time import sleep

from wcferry import Wcf


def main():
    wcf = Wcf()

    def handler(sig, frame):
        wcf.cleanup()  # 退出前清理环境
        exit(0)

    signal.signal(signal.SIGINT, handler)
    sleep(1)  # Slow down
    print(f"Is Login: {True if wcf.is_login() else False}")
    print(f"SelfWxid: {wcf.get_self_wxid()}")

    sleep(1)
    wcf.enable_recv_msg(print)
    # wcf.disable_recv_msg()  # 当需要停止接收消息的时候，随时调用

    ret = wcf.send_text("Hello world.", "filehelper")
    print(f"send_text: {ret}")

    ret = wcf.send_image("TEQuant.jpeg", "filehelper")
    print(f"send_image: {ret}")

    print(f"Message types:\n{wcf.get_msg_types()}")
    print(f"Contacts:\n{wcf.get_contacts()}")

    print(f"DBs:\n{wcf.get_dbs()}")
    print(f"Tables:\n{wcf.get_tables('db')}")
    print(f"Results:\n{wcf.query_sql('MicroMsg.db', 'SELECT * FROM Contact LIMIT 1;')}")

    # wcf.accept_new_friend("v3", "v4")  # 需要真正的 V3、V4 信息

    # 阻塞程序，让程序一直运行
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
pip install grpcio grpcio-tools
```

### 重新生成 gRPC 文件
```sh
cd wcf
python -m grpc_tools.protoc --python_out=. --grpc_python_out=. -I=../ wcf.proto
```

### 参考项目 [README](../README.MD)
