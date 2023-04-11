#! /usr/bin/env python3
# -*- coding: utf-8 -*-

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
from google.protobuf import json_format

WCF_ROOT = os.path.abspath(os.path.dirname(__file__))
sys.path.insert(0, WCF_ROOT)
import wcf_pb2  # noqa

__version__ = "3.7.0.30.22"


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
    """WeChatFerry, a tool to play WeChat."""
    class WxMsg():
        """微信消息"""

        def __init__(self, msg: wcf_pb2.WxMsg) -> None:
            self._is_self = msg.is_self
            self._is_group = msg.is_group
            self.type = msg.type
            self.id = msg.id
            self.xml = msg.xml
            self.sender = msg.sender
            self.roomid = msg.roomid
            self.content = msg.content
            self.thumb = msg.thumb
            self.extra = msg.extra

        def __str__(self) -> str:
            s = f"{'自己发的:' if self._is_self else ''}"
            s += f"{self.sender}[{self.roomid}]:{self.id}:{self.type}:{self.xml.replace(chr(10), '').replace(chr(9),'')}\n"
            s += self.content
            s += f"\n{self.thumb}" if self.thumb else ""
            s += f"\n{self.extra}" if self.extra else ""
            return s

        def from_self(self) -> bool:
            """是否自己发的消息"""
            return self._is_self == 1

        def from_group(self) -> bool:
            """是否群聊消息"""
            return self._is_group

        def is_at(self, wxid) -> bool:
            """是否被@：群消息，在@名单里，并且不是@所有人"""
            return self.from_group() and re.findall(
                f"<atuserlist>.*({wxid}).*</atuserlist>", self.xml) and not re.findall(r"@(?:所有人|all)", self.xml)

        def is_text(self) -> bool:
            """是否文本消息"""
            return self.type == 1

    def __init__(self, host: str = None, port: int = 10086, debug: bool = True) -> None:
        self._local_host = False
        self._is_running = False
        self._is_receiving_msg = False
        self.LOG = logging.getLogger("WCF")
        self.LOG.info(f"wcferry version: {__version__}")
        self.port = port
        self.host = host
        if host is None:
            self._local_host = True
            self.host = "127.0.0.1"
            cmd = f"{WCF_ROOT}/wcf.exe start {self.port} {'debug' if debug else ''}"
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

        if self._local_host:
            cmd = f"{WCF_ROOT}/wcf.exe stop"
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
        """获取 db 中所有表"""
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_GET_DB_TABLES  # FUNC_GET_DB_TABLES
        req.str = db
        rsp = self._send_request(req)
        tables = json_format.MessageToDict(rsp.tables).get("tables", [])

        return tables

    def get_user_info(self) -> dict:
        """获取个人信息"""
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_GET_USER_INFO  # FUNC_GET_USER_INFO
        rsp = self._send_request(req)
        ui = json_format.MessageToDict(rsp.ui)

        return ui

    def send_text(self, msg: str, receiver: str, aters: Optional[str] = "") -> int:
        """发送文本消息"""
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_SEND_TXT  # FUNC_SEND_TXT
        req.txt.msg = msg
        req.txt.receiver = receiver
        if aters:
            req.txt.aters = aters
        rsp = self._send_request(req)
        return rsp.status

    def send_image(self, path: str, receiver: str) -> int:
        """发送图片"""
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_SEND_IMG  # FUNC_SEND_IMG
        req.file.path = path
        req.file.receiver = receiver
        rsp = self._send_request(req)
        return rsp.status

    def send_file(self, path: str, receiver: str) -> int:
        """发送文件"""
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_SEND_FILE  # FUNC_SEND_FILE
        req.file.path = path
        req.file.receiver = receiver
        rsp = self._send_request(req)
        return rsp.status

    def send_xml(self, receiver: str, xml: str, type: int, path: str = None) -> int:
        """发送文件"""
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
        """发送表情"""
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_SEND_EMOTION  # FUNC_SEND_EMOTION
        req.file.path = path
        req.file.receiver = receiver
        rsp = self._send_request(req)
        return rsp.status

    def get_msg(self, block=True) -> WxMsg:
        return self.msgQ.get(block, timeout=1)

    def enable_receiving_msg(self) -> bool:
        """允许接收消息"""
        def listening_msg():
            rsp = wcf_pb2.Response()
            self.msg_socket.dial(self.msg_url, block=True)
            while self._is_receiving_msg:
                try:
                    rsp.ParseFromString(self.msg_socket.recv_msg().bytes)
                except Exception as e:
                    pass
                else:
                    self.msgQ.put(self.WxMsg(rsp.wxmsg))

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
        """设置接收消息回调"""
        def listening_msg():
            rsp = wcf_pb2.Response()
            self.msg_socket.dial(self.msg_url, block=True)
            while self._is_receiving_msg:
                try:
                    rsp.ParseFromString(self.msg_socket.recv_msg().bytes)
                except Exception as e:
                    pass
                else:
                    callback(self.WxMsg(rsp.wxmsg))
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
        """执行 SQL"""
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

    def accept_new_friend(self, v3: str, v4: str) -> int:
        """添加好友"""
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_ACCEPT_FRIEND  # FUNC_ACCEPT_FRIEND
        req.v.v3 = v3
        req.v.v4 = v4
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
        """添加群成员"""
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_ADD_ROOM_MEMBERS  # FUNC_ADD_ROOM_MEMBERS
        req.m.roomid = roomid
        req.m.wxids = wxids
        rsp = self._send_request(req)
        return rsp.status
