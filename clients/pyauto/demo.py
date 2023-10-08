#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
from time import sleep
from wcfauto import Register, Wcf, WxMsg

logging.basicConfig(level='DEBUG', format="%(asctime)s %(message)s")
LOG = logging.getLogger("Demo")


def main():
    receiver = Register()

    @receiver.message_register(isDivision=True, isGroup=True, isPyq=False)
    def process_msg(bot: Wcf, msg: WxMsg):
        """
        同步消息函数装饰器
        """
        LOG.info(f"收到消息: {msg}")

        sleep(5)  # 等微信加载好，以免信息显示异常
        LOG.info(f"已经登录: {True if bot.is_login() else False}")
        LOG.info(f"wxid: {bot.get_self_wxid()}")

        # bot.disable_recv_msg() # 当需要停止接收消息时调用
        sleep(5)
        ret = bot.send_text("Hello world.", "filehelper")
        LOG.info(f"send_text: {ret}")

        sleep(5)
        # 需要确保图片路径正确，建议使用绝对路径（使用双斜杠\\）
        ret = bot.send_image(
            "https://raw.githubusercontent.com/lich0821/WeChatFerry/master/assets/QR.jpeg", "filehelper")
        LOG.info(f"send_image: {ret}")

        sleep(5)
        # 需要确保文件路径正确，建议使用绝对路径（使用双斜杠\\）
        ret = bot.send_file("https://raw.githubusercontent.com/lich0821/WeChatFerry/master/README.MD", "filehelper")
        LOG.info(f"send_file: {ret}")

        sleep(5)
        LOG.info(f"Message types:\n{bot.get_msg_types()}")
        LOG.info(f"Contacts:\n{bot.get_contacts()}")

        sleep(5)
        LOG.info(f"DBs:\n{bot.get_dbs()}")
        LOG.info(f"Tables:\n{bot.get_tables('db')}")
        LOG.info(f"Results:\n{bot.query_sql('MicroMsg.db', 'SELECT * FROM Contact LIMIT 1;')}")

        # 需要真正的 V3、V4 信息
        # bot.accept_new_friend("v3", "v4")

        # 添加群成员，填写正确的群 ID 和成员 wxid
        # ret = bot.add_chatroom_members("chatroom id", "wxid1,wxid2,wxid3,...")
        # LOG.info(f"add_chatroom_members: {ret}")

        # 删除群成员，填写正确的群 ID 和成员 wxid
        # ret = bot.del_chatroom_members("chatroom id", "wxid1,wxid2,wxid3,...")
        # LOG.info(f"add_chatroom_members: {ret}")

        sleep(5)
        bot.refresh_pyq(0)  # 刷新朋友圈第一页
        # bot.refresh_pyq(id)  # 从 id 开始刷新朋友圈

    @receiver.async_message_register()
    async def async_process_msg(bot: Wcf, msg: WxMsg):
        """
        异步消息函数装饰器
        """
        print(msg)

    @receiver.group_changed_register(allow_other_receive=False)
    async def group_changed(bot: Wcf, msg: WxMsg):
        """
        群组信息变化函数装饰器
        """
        print(msg)

    @receiver.revoke_message_register(allow_other_receive=False)
    async def group_changed(bot: Wcf, msg: WxMsg):
        """
        撤回消息函数装饰器
        """
        print(msg)
        print(msg.get_revoke_msg())

    def judge(msg: WxMsg):
        """
        消息判断函数
        """
        return False


    @receiver.custom_message_register(register_name='custom', msg_judge_func=judge, allow_other_receive=False)
    async def group_changed(bot: Wcf, msg: WxMsg):
        """
        自定义消息接收函数装饰器
        """
        print(msg)

    # 开始接受消息
    receiver.run()


if __name__ == "__main__":
    main()
