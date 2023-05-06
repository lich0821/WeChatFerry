#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
from typing import Any

import requests
from fastapi import Body, FastAPI
from pydantic import BaseModel
from wcferry import Wcf, WxMsg

__version__ = "3.7.0.30.25"


class Msg(BaseModel):
    id: str
    type: int
    xml: str
    sender: str
    roomid: str
    content: str
    thumb: str
    extra: str
    is_self: bool
    is_group: bool


class Http(FastAPI):
    """WeChatFerry HTTP 客户端，文档地址：http://IP:PORT/docs"""

    def __init__(self, wcf: Wcf, cb: str, **extra: Any) -> None:
        super().__init__(**extra)
        self.LOG = logging.getLogger(__name__)
        self.wcf = wcf
        self._set_cb(cb)
        self.add_api_route("/msg_cb", self.msg_cb, methods=["POST"], summary="接收消息回调样例")
        self.add_api_route("/text", self.send_text, methods=["POST"], summary="发送文本消息")

    def _set_cb(self, cb):
        def callback(msg: WxMsg):
            data = {}
            data["id"] = msg.id
            data["type"] = msg.type
            data["xml"] = msg.xml
            data["sender"] = msg.sender
            data["roomid"] = msg.roomid
            data["content"] = msg.content
            data["thumb"] = msg.thumb
            data["extra"] = msg.extra
            data["is_self"] = msg.from_self()
            data["is_group"] = msg.from_group()

            try:
                rsp = requests.post(url=cb, json=data)
                if rsp.status_code != 200:
                    self.LOG.error(f"消息转发失败，HTTP 状态码为: {rsp.status_code}")
            except Exception as e:
                self.LOG.error(f"消息转发异常: {e}")

        if cb:
            self.LOG.info(f"消息回调: {cb}")
            self.wcf.enable_recv_msg(callback=callback)
        else:
            self.LOG.info(f"没有设置回调，打印消息")
            self.wcf.enable_recv_msg(print)

    def msg_cb(self, msg: Msg):
        """示例回调方法，简单打印消息"""
        print(f"收到消息：{msg}")
        return {"status": 0, "message": "成功"}

    def send_text(self, msg: str = Body("消息"), receiver: str = Body("filehelper"), aters: str = Body("")) -> dict:
        """发送文本消息，可参考：robot.py 里 sendTextMsg"""
        ret = self.wcf.send_text(msg, receiver, aters)
        return {"status": ret, "message": "成功"if ret == 0 else "失败"}
