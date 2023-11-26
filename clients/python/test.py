#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
from queue import Empty
from threading import Thread
from time import sleep

from wcferry import Wcf

logging.basicConfig(level='DEBUG', format="%(asctime)s [%(levelname)s] %(message)s", datefmt="%Y-%m-%d %H:%M:%S")
LOG = logging.getLogger("Demo")


def process_msg(wcf: Wcf):
    """处理接收到的消息"""
    while wcf.is_receiving_msg():
        try:
            msg = wcf.get_msg()
            LOG.info(msg)  # 简单打印
        except Empty:
            continue  # Empty message
        except Exception as e:
            LOG.error(f"Receiving message error: {e}")


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

    return wcf


if __name__ == "__main__":
    wcf = main()

    # 一直运行
    wcf.keep_running()
