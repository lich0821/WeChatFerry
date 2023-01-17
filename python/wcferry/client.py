#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import atexit
import ctypes
import logging
import os
import re
import sys
from threading import Thread
from time import sleep
from typing import List, Callable, Optional

import grpc

WCF_ROOT = os.path.abspath(os.path.dirname(__file__))
sys.path.insert(0, WCF_ROOT)
import wcf_pb2       # noqa
import wcf_pb2_grpc  # noqa

__version__ = "v3.7.0.30.12"


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

        def __str__(self) -> str:
            s = f"{self.sender}[{self.roomid}]\t{self.id}-{self.type}-{self.xml.replace(chr(10), '').replace(chr(9),'')}\n"
            s += self.content
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

    def __init__(self, host_port: str = None) -> None:
        self._local_host = False
        self._is_running = False
        self._enable_recv_msg = False
        self.LOG = logging.getLogger("WCF")
        if host_port is None:
            self._local_host = True
            host_port = "127.0.0.1:10086"
            self._sdk = ctypes.cdll.LoadLibrary(f"{WCF_ROOT}/sdk.dll")
            if self._sdk.WxInitSDK() != 0:
                self.LOG.error("初始化失败！")

        self._channel = grpc.insecure_channel(host_port)
        self._stub = wcf_pb2_grpc.WcfStub(self._channel)
        atexit.register(self.disable_recv_msg)  # 退出的时候停止消息接收，防止内存泄露
        self._is_running = True
        self.contacts = []
        self._SQL_TYPES = {1: int, 2: float, 3: lambda x: x.decode("utf-8"), 4: bytes, 5: lambda x: None}
        self.self_wxid = self.get_self_wxid()

    def __del__(self) -> None:
        self.cleanup()

    def cleanup(self) -> None:
        """停止 gRPC，关闭连接，回收资源"""
        if not self._is_running:
            return

        self.disable_recv_msg()
        self._channel.close()
        if self._local_host:
            self._sdk.WxDestroySDK()
            handle = self._sdk._handle
            del self._sdk
            ctypes.windll.kernel32.FreeLibrary(handle)
        self._is_running = False

    def keep_running(self):
        """阻塞进程，让 RPC 一直维持连接"""
        try:
            while True:
                sleep(1)
        except Exception as e:
            self.cleanup()

    def is_login(self) -> int:
        """是否已经登录"""
        rsp = self._stub.RpcIsLogin(wcf_pb2.Empty())
        return rsp.status

    def get_self_wxid(self) -> str:
        """获取登录账户的 wxid"""
        rsp = self._stub.RpcGetSelfWxid(wcf_pb2.Empty())
        return rsp.str

    def _rpc_get_message(self, func):
        rsps = self._stub.RpcEnableRecvMsg(wcf_pb2.Empty())
        try:
            for rsp in rsps:
                func(self.WxMsg(rsp))
        except Exception as e:
            self.LOG.error(f"RpcEnableRecvMsg: {e}")
        finally:
            self.disable_recv_msg()

    def enable_recv_msg(self, callback: Callable[[WxMsg], None] = None) -> bool:
        """设置接收消息回调"""
        if self._enable_recv_msg:
            return True

        if callback is None:
            return False

        self._enable_recv_msg = True
        # 阻塞，把控制权交给用户
        # self._rpc_get_message(callback)

        # 不阻塞，启动一个新的线程来接收消息
        Thread(target=self._rpc_get_message, name="GetMessage", args=(callback,), daemon=True).start()

        return True

    def disable_recv_msg(self) -> int:
        """停止接收消息"""
        if not self._enable_recv_msg:
            return -1

        rsp = self._stub.RpcDisableRecvMsg(wcf_pb2.Empty())
        if rsp.status == 0:
            self._enable_recv_msg = False

        return rsp.status

    def send_text(self, msg: str, receiver: str, aters: Optional[str] = "") -> int:
        """发送文本消息"""
        rsp = self._stub.RpcSendTextMsg(wcf_pb2.TextMsg(msg=msg, receiver=receiver, aters=aters))
        return rsp.status

    def send_image(self, path: str, receiver: str) -> int:
        """发送图片"""
        rsp = self._stub.RpcSendImageMsg(wcf_pb2.ImageMsg(path=path, receiver=receiver))
        return rsp.status

    def get_msg_types(self) -> dict:
        """获取所有消息类型"""
        rsp = self._stub.RpcGetMsgTypes(wcf_pb2.Empty())
        return dict(sorted(dict(rsp.types).items()))

    def get_contacts(self) -> List[dict]:
        """获取完整通讯录"""
        rsp = self._stub.RpcGetContacts(wcf_pb2.Empty())
        for cnt in rsp.contacts:
            gender = ""
            if cnt.gender == 1:
                gender = "男"
            elif cnt.gender == 2:
                gender = "女"
            self.contacts.append({"wxid": cnt.wxid, "code": cnt.code, "name": cnt.name,
                                 "country": cnt.country, "province": cnt.province, "city": cnt.city, "gender": gender})
        return self.contacts

    def get_dbs(self) -> List[str]:
        """获取所有数据库"""
        rsp = self._stub.RpcGetDbNames(wcf_pb2.Empty())
        return rsp.names

    def get_tables(self, db: str) -> List[dict]:
        """获取 db 中所有表"""
        tables = []
        rsp = self._stub.RpcGetDbTables(wcf_pb2.String(str=db))
        for tbl in rsp.tables:
            tables.append({"name": tbl.name, "sql": tbl.sql})
        return tables

    def query_sql(self, db: str, sql: str) -> List[dict]:
        """执行 SQL"""
        result = []
        rsp = self._stub.RpcExecDbQuery(wcf_pb2.DbQuery(db=db, sql=sql))
        for r in rsp.rows:
            row = {}
            for f in r.fields:
                row[f.column] = self._SQL_TYPES[f.type](f.content)
            result.append(row)
        return result

    def accept_new_friend(self, v3: str, v4: str) -> int:
        """通过好友验证"""
        rsp = self._stub.RpcAcceptNewFriend(wcf_pb2.Verification(v3=v3, v4=v4))
        return rsp.status
