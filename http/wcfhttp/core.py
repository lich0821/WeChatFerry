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
        self.add_api_route("/image", self.send_image, methods=["POST"], summary="发送图片消息")
        self.add_api_route("/file", self.send_file, methods=["POST"], summary="发送文件消息")
        self.add_api_route("/xml", self.send_xml, methods=["POST"], summary="发送 XML 消息")
        self.add_api_route("/emotion", self.send_emotion, methods=["POST"], summary="发送表情消息")
        self.add_api_route("/login", self.is_login, methods=["GET"], summary="获取登录状态")
        self.add_api_route("/wxid", self.get_self_wxid, methods=["GET"], summary="获取登录账号 wxid")

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

    def send_image(self,
                   path: str = Body("C:\\Projs\\WeChatRobot\\TEQuant.jpeg", description="图片路径"),
                   receiver: str = Body("filehelper", description="roomid 或者 wxid")) -> dict:
        """发送图片消息"""
        ret = self.wcf.send_image(path, receiver)
        return {"status": ret, "message": "成功"if ret == 0 else "失败"}

    def send_file(self,
                  path: str = Body("C:\\Projs\\WeChatRobot\\TEQuant.jpeg", description="本地文件路径，不支持网络路径"),
                  receiver: str = Body("filehelper", description="roomid 或者 wxid")) -> dict:
        """发送文件消息"""
        ret = self.wcf.send_file(path, receiver)
        return {"status": ret, "message": "成功"if ret == 0 else "失败"}

    def send_xml(
            self, receiver: str = Body("filehelper", description="roomid 或者 wxid"),
            xml:
            str = Body(
                '<?xml version="1.0"?><msg><appmsg appid="" sdkver="0"><title>叮当药房，24小时服务，28分钟送药到家！</title><des>叮当快药首家承诺范围内28分钟送药到家！叮当快药核心区域内7*24小时全天候服务，送药上门！叮当快药官网为您提供快捷便利，正品低价，安全放心的购药、送药服务体验。</des><action>view</action><type>33</type></appmsg><fromusername>wxid_xxxxxxxxxxxxxx</fromusername><scene>0</scene><appinfo><version>1</version><appname /></appinfo><commenturl /></msg>',
                description="xml 内容"),
            type: int = Body(0x21, description="xml 类型，0x21 为小程序"),
            path: str = Body(None, description="封面图片路径")) -> dict:
        """发送 XML 消息"""
        ret = self.wcf.send_xml(receiver, xml, type, path)
        return {"status": ret, "message": "成功"if ret == 0 else "失败"}

    def send_emotion(self,
                     path: str = Body("C:/Projs/WeChatRobot/emo.gif", description="本地文件路径，不支持网络路径"),
                     receiver: str = Body("filehelper", description="roomid 或者 wxid")) -> dict:
        """发送表情消息"""
        ret = self.wcf.send_emotion(path, receiver)
        return {"status": ret, "message": "成功"if ret == 0 else "失败"}

    def is_login(self) -> dict:
        """获取登录状态"""
        ret = self.wcf.is_login()
        return {"status": ret, "message": "成功", "data": {"login": ret}}

    def get_self_wxid(self) -> dict:
        """获取登录状态"""
        ret = self.wcf.get_self_wxid()
        return {"status": ret, "message": "成功", "data": {"wxid": ret}}
