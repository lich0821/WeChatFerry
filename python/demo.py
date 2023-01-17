#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
import signal
from time import sleep

from wcferry import Wcf


def main():
    logging.info("Start demo...")
    wcf = Wcf()             # 默认连接本地服务
    # wcf = Wcf("IP:10086") # 连接远端服务

    def handler(sig, frame):
        wcf.cleanup()
        exit(0)

    signal.signal(signal.SIGINT, handler)
    sleep(1)  # Slow down
    print(f"Is Login: {True if wcf.is_login() else False}")
    print(f"SelfWxid: {wcf.get_self_wxid()}")

    sleep(1)
    wcf.enable_recv_msg(print)
    # wcf.disable_recv_msg() # Call anytime when you don't want to receive messages

    ret = wcf.send_text("Hello world.", "filehelper")
    print(f"send_text: {ret}")

    ret = wcf.send_image("TEQuant.jpeg", "filehelper")
    print(f"send_image: {ret}")

    print(f"Message types:\n{wcf.get_msg_types()}")
    print(f"Contacts:\n{wcf.get_contacts()}")

    print(f"DBs:\n{wcf.get_dbs()}")
    print(f"Tables:\n{wcf.get_tables('db')}")
    print(f"Results:\n{wcf.query_sql('MicroMsg.db', 'SELECT * FROM Contact LIMIT 1;')}")

    # wcf.accept_new_friend("v3", "v4") # 需要真正的 V3、V4 信息

    # Keep running to receive messages
    wcf.keep_running()


if __name__ == "__main__":
    logging.basicConfig(level='DEBUG')
    main()
