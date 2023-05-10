#! /usr/bin/env python3
# -*- coding: utf-8 -*-

__version__ = "37.1.25.1"

import atexit
import base64
import logging
import os
import re
import sys
from queue import Queue
from threading import Thread
from time import sleep
from typing import Callable, List, Optional

import pynng
import requests
from google.protobuf import json_format
from wcferry import wcf_pb2
from wcferry.wxmsg import WxMsg


def _retry():
    def decorator(func):
        """ Retry the function """
        def wrapper(*args, **kwargs):
            def logerror(e):
                func_name = re.findall(r"func: (.*?)\n", str(args[1]))[-1]
                logging.getLogger("WCF").error(f"Call {func_name} failed: {e}")

            try:
                ret = func(*args, **kwargs)
            except pynng.Timeout as _:  # 如果超时，重试
                try:
                    ret = func(*args, **kwargs)
                except Exception as e:
                    logerror(e)
                    ret = wcf_pb2.Response()
            except Exception as e:  # 其他异常，退出
                logerror(e)
                sys.exit(-1)

            return ret
        return wrapper
    return decorator


class Wcf():
    """WeChatFerry, 一个玩微信的工具。

    Args:
        host (str): `wcferry` RPC 服务器地址，默认本地启动；也可以指定地址连接远程服务
        port (int): `wcferry` RPC 服务器端口，默认为 10086，接收消息会占用 `port+1` 端口
        debug (bool): 是否开启调试模式（仅本地启动有效）

    Attributes:
        contacts (list): 联系人缓存，调用 `get_contacts` 后更新
        self_wxid (str): 登录账号 wxid
    """

    def __init__(self, host: str = None, port: int = 10086, debug: bool = True) -> None:
        self._local_mode = False
        self._is_running = False
        self._is_receiving_msg = False
        self._wcf_root = os.path.abspath(os.path.dirname(__file__))
        self.LOG = logging.getLogger("WCF")
        self.LOG.info(f"wcferry version: {__version__}")
        self.port = port
        self.host = host
        if host is None:
            self._local_mode = True
            self.host = "127.0.0.1"
            cmd = fr'"{self._wcf_root}\wcf.exe" start {self.port} {"debug" if debug else ""}'
            if os.system(cmd) != 0:
                self.LOG.error("初始化失败！")
                os._exit(-1)

        self.cmd_url = f"tcp://{self.host}:{self.port}"

        # 连接 RPC
        self.cmd_socket = pynng.Pair1()  # Client --> Server，发送消息
        self.cmd_socket.send_timeout = 2000  # 发送 2 秒超时
        self.cmd_socket.recv_timeout = 2000  # 接收 2 秒超时
        try:
            self.cmd_socket.dial(self.cmd_url, block=True)
        except Exception as e:
            self.LOG.error(f"连接失败: {e}")
            os._exit(-2)

        self.msg_socket = pynng.Pair1()  # Server --> Client，接收消息
        self.msg_socket.send_timeout = 2000  # 发送 2 秒超时
        self.msg_socket.recv_timeout = 2000  # 接收 2 秒超时
        self.msg_url = self.cmd_url.replace(str(self.port), str(self.port + 1))

        atexit.register(self.cleanup)  # 退出的时候停止消息接收，防止资源占用
        while not self.is_login():     # 等待微信登录成功
            sleep(1)

        self._is_running = True
        self.contacts = []
        self.msgQ = Queue()
        self._SQL_TYPES = {1: int, 2: float, 3: lambda x: x.decode("utf-8"), 4: bytes, 5: lambda x: None}
        self.self_wxid = self.get_self_wxid()

    def __del__(self) -> None:
        self.cleanup()

    def cleanup(self) -> None:
        """关闭连接，回收资源"""
        if not self._is_running:
            return

        self.disable_recv_msg()
        self.cmd_socket.close()

        if self._local_mode:
            cmd = fr'"{self._wcf_root}\wcf.exe" stop'
            if os.system(cmd) != 0:
                self.LOG.error("退出失败！")
                return
        self._is_running = False

    def keep_running(self):
        """阻塞进程，让 RPC 一直维持连接"""
        try:
            while True:
                sleep(1)
        except Exception as e:
            self.cleanup()

    @_retry()
    def _send_request(self, req: wcf_pb2.Request) -> wcf_pb2.Response:
        data = req.SerializeToString()
        self.cmd_socket.send(data)
        rsp = wcf_pb2.Response()
        rsp.ParseFromString(self.cmd_socket.recv_msg().bytes)
        return rsp

    def is_receiving_msg(self) -> bool:
        """是否已启动接收消息功能"""
        return self._is_receiving_msg

    def is_login(self) -> bool:
        """是否已经登录"""
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_IS_LOGIN  # FUNC_IS_LOGIN
        rsp = self._send_request(req)

        return rsp.status == 1

    def get_self_wxid(self) -> str:
        """获取登录账户的 wxid"""
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_GET_SELF_WXID  # FUNC_GET_SELF_WXID
        rsp = self._send_request(req)

        return rsp.str

    def get_msg_types(self) -> dict:
        """获取所有消息类型"""
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_GET_MSG_TYPES  # FUNC_GET_MSG_TYPES
        rsp = self._send_request(req)
        types = json_format.MessageToDict(rsp.types).get("types", {})
        types = {int(k): v for k, v in types.items()}

        return dict(sorted(dict(types).items()))

    def get_contacts(self) -> List[dict]:
        """获取完整通讯录"""
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_GET_CONTACTS  # FUNC_GET_CONTACTS
        rsp = self._send_request(req)
        contacts = json_format.MessageToDict(rsp.contacts).get("contacts", [])

        for cnt in contacts:
            gender = cnt.get("gender", "")
            if gender == 1:
                gender = "男"
            elif gender == 2:
                gender = "女"
            contact = {
                "wxid": cnt.get("wxid", ""),
                "code": cnt.get("code", ""),
                "remark": cnt.get("remark", ""),
                "name": cnt.get("name", ""),
                "country": cnt.get("country", ""),
                "province": cnt.get("province", ""),
                "city": cnt.get("city", ""),
                "gender": gender}
            self.contacts.append(contact)
        return self.contacts

    def get_dbs(self) -> List[str]:
        """获取所有数据库"""
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_GET_DB_NAMES  # FUNC_GET_DB_NAMES
        rsp = self._send_request(req)
        dbs = json_format.MessageToDict(rsp.dbs).get("names", [])

        return dbs

    def get_tables(self, db: str) -> List[dict]:
        """获取 db 中所有表

        Args:
            db (str): 数据库名（可通过 `get_dbs` 查询）

        Returns:
            List[dict]: `db` 下的所有表名及对应建表语句
        """
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_GET_DB_TABLES  # FUNC_GET_DB_TABLES
        req.str = db
        rsp = self._send_request(req)
        tables = json_format.MessageToDict(rsp.tables).get("tables", [])

        return tables

    def get_user_info(self) -> dict:
        """获取登录账号个人信息"""
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_GET_USER_INFO  # FUNC_GET_USER_INFO
        rsp = self._send_request(req)
        ui = json_format.MessageToDict(rsp.ui)

        return ui

    def send_text(self, msg: str, receiver: str, aters: Optional[str] = "") -> int:
        """发送文本消息

        Args:
            msg (str): 要发送的消息，换行使用 `\\n`；如果 @ 人的话，需要带上跟 `aters` 里数量相同的 @
            receiver (str): 消息接收人，wxid 或者 roomid
            aters (str): 要 @ 的 wxid，多个用逗号分隔；`@所有人` 只需要 `nofity@all`

        Returns:
            int: 0 为成功，其他失败
        """
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_SEND_TXT  # FUNC_SEND_TXT
        req.txt.msg = msg
        req.txt.receiver = receiver
        if aters:
            req.txt.aters = aters
        rsp = self._send_request(req)
        return rsp.status

    def send_image(self, path: str, receiver: str) -> int:
        """发送图片，非线程安全

        Args:
            path (str): 图片路径，如：`C:/Projs/WeChatRobot/TEQuant.jpeg` 或 `https://raw.githubusercontent.com/lich0821/WeChatRobot/master/TEQuant.jpeg`
            receiver (str): 消息接收人，wxid 或者 roomid

        Returns:
            int: 0 为成功，其他失败
        """
        if path.startswith("http"):
            if not self._local_mode:
                self.LOG.error(f"只有本地模式才支持网络路径！")
                return -1
            try:
                headers = {
                    'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/100.0.4896.127 Safari/537.36', }
                response = requests.get(path, headers=headers, stream=True)
                response.raw.decode_content = True

                # 保存图片，不删除，等下次覆盖
                with open(f"{self._wcf_root}/.tmp.jpg", "wb") as of:
                    of.write(response.content)

                path = f"{self._wcf_root}/.tmp.jpg"
            except Exception as e:
                self.LOG.error(e)
                return -1

        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_SEND_IMG  # FUNC_SEND_IMG
        req.file.path = path
        req.file.receiver = receiver
        rsp = self._send_request(req)
        return rsp.status

    def send_file(self, path: str, receiver: str) -> int:
        """发送文件

        Args:
            path (str): 本地文件路径，如：`C:/Projs/WeChatRobot/README.MD`
            receiver (str): 消息接收人，wxid 或者 roomid

        Returns:
            int: 0 为成功，其他失败
        """
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_SEND_FILE  # FUNC_SEND_FILE
        req.file.path = path
        req.file.receiver = receiver
        rsp = self._send_request(req)
        return rsp.status

    def send_xml(self, receiver: str, xml: str, type: int, path: str = None) -> int:
        """发送 XML

        Args:
            receiver (str): 消息接收人，wxid 或者 roomid
            xml (str): xml 内容
            type (int): xml 类型，如：0x21 为小程序
            path (str): 封面图片路径

        Returns:
            int: 0 为成功，其他失败
        """
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_SEND_XML  # FUNC_SEND_XML
        req.xml.receiver = receiver
        req.xml.content = xml
        req.xml.type = type
        if path:
            req.xml.path = path
        rsp = self._send_request(req)
        return rsp.status

    def send_emotion(self, path: str, receiver: str) -> int:
        """发送表情

        Args:
            path (str): 本地表情路径，如：`C:/Projs/WeChatRobot/emo.gif`
            receiver (str): 消息接收人，wxid 或者 roomid

        Returns:
            int: 0 为成功，其他失败
        """
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_SEND_EMOTION  # FUNC_SEND_EMOTION
        req.file.path = path
        req.file.receiver = receiver
        rsp = self._send_request(req)
        return rsp.status

    def get_msg(self, block=True) -> WxMsg:
        """从消息队列中获取消息

        Args:
            block (bool): 是否阻塞，默认阻塞

        Returns:
            WxMsg: 微信消息

        Raises:
            Empty: 如果阻塞并且超时，抛出空异常，需要用户自行捕获
        """
        return self.msgQ.get(block, timeout=1)

    def enable_receiving_msg(self) -> bool:
        """允许接收消息，成功后通过 `get_msg` 读取消息"""
        def listening_msg():
            rsp = wcf_pb2.Response()
            self.msg_socket.dial(self.msg_url, block=True)
            while self._is_receiving_msg:
                try:
                    rsp.ParseFromString(self.msg_socket.recv_msg().bytes)
                except Exception as e:
                    pass
                else:
                    self.msgQ.put(WxMsg(rsp.wxmsg))

            # 退出前关闭通信通道
            self.msg_socket.close()

        if self._is_receiving_msg:
            return True

        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_ENABLE_RECV_TXT  # FUNC_ENABLE_RECV_TXT
        rsp = self._send_request(req)
        if rsp.status != 0:
            return False

        self._is_receiving_msg = True
        # 阻塞，把控制权交给用户
        # self._rpc_get_message(callback)

        # 不阻塞，启动一个新的线程来接收消息
        Thread(target=listening_msg, name="GetMessage", daemon=True).start()

        return True

    def enable_recv_msg(self, callback: Callable[[WxMsg], None] = None) -> bool:
        """（不建议使用）设置接收消息回调，消息量大时可能会丢失消息

        .. deprecated:: 3.7.0.30.13
        """
        def listening_msg():
            rsp = wcf_pb2.Response()
            self.msg_socket.dial(self.msg_url, block=True)
            while self._is_receiving_msg:
                try:
                    rsp.ParseFromString(self.msg_socket.recv_msg().bytes)
                except Exception as e:
                    pass
                else:
                    callback(WxMsg(rsp.wxmsg))
            # 退出前关闭通信通道
            self.msg_socket.close()

        if self._is_receiving_msg:
            return True

        if callback is None:
            return False

        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_ENABLE_RECV_TXT  # FUNC_ENABLE_RECV_TXT
        rsp = self._send_request(req)
        if rsp.status != 0:
            return False

        self._is_receiving_msg = True
        # 阻塞，把控制权交给用户
        # listening_msg()

        # 不阻塞，启动一个新的线程来接收消息
        Thread(target=listening_msg, name="GetMessage", daemon=True).start()

        return True

    def disable_recv_msg(self) -> int:
        """停止接收消息"""
        if not self._is_receiving_msg:
            return 0

        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_DISABLE_RECV_TXT  # FUNC_DISABLE_RECV_TXT
        rsp = self._send_request(req)
        self._is_receiving_msg = False

        return rsp.status

    def query_sql(self, db: str, sql: str) -> List[dict]:
        """执行 SQL，如果数据量大注意分页，以免 OOM

        Args:
            db (str): 要查询的数据库
            sql (str): 要执行的 SQL

        Returns:
            List[dict]: 查询结果
        """
        result = []
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_EXEC_DB_QUERY  # FUNC_EXEC_DB_QUERY
        req.query.db = db
        req.query.sql = sql
        rsp = self._send_request(req)
        rows = json_format.MessageToDict(rsp.rows).get("rows", [])
        for r in rows:
            row = {}
            for f in r["fields"]:
                c = base64.b64decode(f.get("content", ""))
                row[f["column"]] = self._SQL_TYPES[f["type"]](c)
            result.append(row)
        return result

    def accept_new_friend(self, v3: str, v4: str, scene: int = 30) -> int:
        """通过好友申请

        Args:
            v3 (str): 加密用户名 (好友申请消息里 v3 开头的字符串)
            v4 (str): Ticket (好友申请消息里 v4 开头的字符串)
            scene: 申请方式 (好友申请消息里的 scene); 为了兼容旧接口，默认为扫码添加 (30)

        Returns:
            int: 1 为成功，其他失败
        """
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_ACCEPT_FRIEND  # FUNC_ACCEPT_FRIEND
        req.v.v3 = v3
        req.v.v4 = v4
        req.v.scene = scene
        rsp = self._send_request(req)
        return rsp.status

    def get_friends(self) -> List[dict]:
        """获取好友列表"""
        not_friends = {
            "fmessage": "朋友推荐消息",
            "medianote": "语音记事本",
            "floatbottle": "漂流瓶",
            "filehelper": "文件传输助手",
            "newsapp": "新闻",
        }
        friends = []
        for cnt in self.get_contacts():
            if (cnt["wxid"].endswith("@chatroom")      # 群聊
                or cnt["wxid"].startswith("gh_")       # 公众号
                or cnt["wxid"] in not_friends.keys()   # 其他杂号
                ):
                continue
            friends.append(cnt)

        return friends

    def add_chatroom_members(self, roomid: str, wxids: str) -> int:
        """添加群成员

        Args:
            roomid (str): 待加群的 id
            wxids (str): 要加到群里的 wxid，多个用逗号分隔

        Returns:
            int: 1 为成功，其他失败
        """
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_ADD_ROOM_MEMBERS  # FUNC_ADD_ROOM_MEMBERS
        req.m.roomid = roomid
        req.m.wxids = wxids
        rsp = self._send_request(req)
        return rsp.status

    def receive_transfer(self, wxid: str, transferid: str) -> int:
        """接收转账

        Args:
            wxid (str): 转账消息里的发送人 wxid
            transferid (str): 转账消息里的 transferid

        Returns:
            int: 1 为成功，其他失败
        """
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_RECV_TRANSFER  # FUNC_RECV_TRANSFER
        req.tf.wxid = wxid
        req.tf.tid = transferid
        rsp = self._send_request(req)
        return rsp.status

    def decrypt_image(self, src: str, dst: str) -> bool:
        """解密图片:

        Args:
            src (str): 加密的图片路径
            dst (str): 解密的图片路径

        Returns:
            bool: 是否成功
        """
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_DECRYPT_IMAGE  # FUNC_DECRYPT_IMAGE
        req.dec.src = src
        req.dec.dst = dst
        rsp = self._send_request(req)
        return rsp.status == 1
