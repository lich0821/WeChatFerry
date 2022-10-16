#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import atexit
import ctypes
import logging
import os
import sys
from threading import Thread
from time import sleep
from typing import Any, Callable, Optional

import grpc

WCF_ROOT = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, WCF_ROOT)
import wcf_pb2       # noqa
import wcf_pb2_grpc  # noqa


class Wcf():
    def __init__(self, host_port: str = "localhost:10086") -> None:
        self._enable_recv_msg = False
        self.LOG = logging.getLogger("WCF")
        self._sdk = ctypes.cdll.LoadLibrary(f"{WCF_ROOT}/sdk.dll")
        if self._sdk.WxInitSDK() != 0:
            self.LOG.error("初始化失败！")
            return

        self._channel = grpc.insecure_channel(host_port)
        self._stub = wcf_pb2_grpc.WcfStub(self._channel)
        atexit.register(self.disable_recv_msg)  # 退出的时候停止消息接收，防止内存泄露
        self._is_running = True

    def __del__(self) -> None:
        self.cleanup()

    def cleanup(self) -> None:
        if not self._is_running:
            return

        self.disable_recv_msg()
        self._channel.close()
        self._sdk.WxDestroySDK()
        handle = self._sdk._handle
        del self._sdk
        ctypes.windll.kernel32.FreeLibrary(handle)
        self._is_running = False

    def keep_running(self):
        try:
            while True:
                sleep(1)
        except Exception as e:
            self.cleanup()

    def is_login(self) -> int:
        rsp = self._stub.RpcIsLogin(wcf_pb2.Empty())
        return rsp.status

    def get_self_wxid(self) -> str:
        rsp = self._stub.RpcGetSelfWxid(wcf_pb2.Empty())
        return rsp.str

    def _rpc_get_message(self, func):
        rsps = self._stub.RpcEnableRecvMsg(wcf_pb2.Empty())
        try:
            for rsp in rsps:
                func(rsp)
        except Exception as e:
            self.LOG.error(f"RpcEnableRecvMsg: {e}")
        finally:
            self.disable_recv_msg()

    def enable_recv_msg(self, callback: Callable[..., Any] = None) -> bool:
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
        if not self._enable_recv_msg:
            return -1

        rsp = self._stub.RpcDisableRecvMsg(wcf_pb2.Empty())
        if rsp.status == 0:
            self._enable_recv_msg = False

        return rsp.status

    def send_text(self, msg: str, receiver: str, aters: Optional[str] = "") -> int:
        rsp = self._stub.RpcSendTextMsg(wcf_pb2.TextMsg(msg=msg, receiver=receiver, aters=aters))
        return rsp.status

    def send_image(self, path: str, receiver: str) -> int:
        rsp = self._stub.RpcSendImageMsg(wcf_pb2.ImageMsg(path=path, receiver=receiver))
        return rsp.status

    def get_msg_types(self) -> wcf_pb2.MsgTypes:
        rsp = self._stub.RpcGetMsgTypes(wcf_pb2.Empty())
        return rsp

    def get_contacts(self) -> wcf_pb2.Contacts:
        rsp = self._stub.RpcGetContacts(wcf_pb2.Empty())
        return rsp

    def get_dbs(self) -> wcf_pb2.DbNames:
        rsp = self._stub.RpcGetDbNames(wcf_pb2.Empty())
        return rsp

    def get_tables(self, db: str) -> wcf_pb2.DbTables:
        rsp = self._stub.RpcGetDbTables(wcf_pb2.String(str=db))
        return rsp

    def query_sql(self, db: str, sql: str) -> wcf_pb2.DbRows:
        rsp = self._stub.RpcExecDbQuery(wcf_pb2.DbQuery(db=db, sql=sql))
        return rsp

    def accept_new_friend(self, v3: str, v4: str) -> int:
        rsp = self._stub.RpcAcceptNewFriend(wcf_pb2.Verification(v3=v3, v4=v4))
        return rsp.status
