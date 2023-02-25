#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import logging

from wcferry import Wcf


def main():
    LOG = logging.getLogger("Demo")
    LOG.info("Start demo...")
    wcf = Wcf(debug=True)             # 默认连接本地服务
    # wcf = Wcf("tcp://127.0.0.1:10086") # 连接远端服务

    LOG.info(f"Is Login: {True if wcf.is_login() else False}")
    LOG.info(f"SelfWxid: {wcf.get_self_wxid()}")

    wcf.enable_recv_msg(LOG.info)
    # wcf.disable_recv_msg() # Call anytime when you don't want to receive messages

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

    # Keep running to receive messages
    wcf.keep_running()


if __name__ == "__main__":
    logging.basicConfig(level='DEBUG', format="%(asctime)s %(message)s")
    main()
