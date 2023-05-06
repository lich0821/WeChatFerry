#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import base64
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

        self.add_api_route("/login", self.is_login, methods=["GET"], summary="获取登录状态")
        self.add_api_route("/wxid", self.get_self_wxid, methods=["GET"], summary="获取登录账号 wxid")
        self.add_api_route("/user-info", self.get_user_info, methods=["GET"], summary="获取登录账号个人信息")
        self.add_api_route("/msg-types", self.get_msg_types, methods=["GET"], summary="获取消息类型")
        self.add_api_route("/contacts", self.get_contacts, methods=["GET"], summary="获取完整通讯录")
        self.add_api_route("/friends", self.get_friends, methods=["GET"], summary="获取好友列表")
        self.add_api_route("/dbs", self.get_dbs, methods=["GET"], summary="获取所有数据库")
        self.add_api_route("/{db}/tables", self.get_tables, methods=["GET"], summary="获取 db 中所有表")

        self.add_api_route("/msg_cb", self.msg_cb, methods=["POST"], summary="接收消息回调样例")
        self.add_api_route("/text", self.send_text, methods=["POST"], summary="发送文本消息")
        self.add_api_route("/image", self.send_image, methods=["POST"], summary="发送图片消息")
        self.add_api_route("/file", self.send_file, methods=["POST"], summary="发送文件消息")
        self.add_api_route("/xml", self.send_xml, methods=["POST"], summary="发送 XML 消息")
        self.add_api_route("/emotion", self.send_emotion, methods=["POST"], summary="发送表情消息")
        self.add_api_route("/sql", self.query_sql, methods=["POST"], summary="执行 SQL，如果数据量大注意分页，以免 OOM")
        self.add_api_route("/new-friend", self.accept_new_friend, methods=["POST"], summary="通过好友申请")
        self.add_api_route("/chatroom-member", self.add_chatroom_members, methods=["POST"], summary="添加群成员")
        self.add_api_route("/transfer", self.receive_transfer, methods=["POST"], summary="接收转账")
        self.add_api_route("/dec-image", self.decrypt_image, methods=["POST"], summary="解密图片")

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

    def is_login(self) -> dict:
        """获取登录状态"""
        ret = self.wcf.is_login()
        return {"status": 0, "message": "成功", "data": {"login": ret}}

    def get_self_wxid(self) -> dict:
        """获取登录账号 wxid"""
        ret = self.wcf.get_self_wxid()
        if ret:
            return {"status": 0, "message": "成功", "data": {"wxid": ret}}
        return {"status": -1, "message": "失败"}

    def get_msg_types(self) -> dict:
        """获取消息类型"""
        ret = self.wcf.get_msg_types()
        if ret:
            return {"status": 0, "message": "成功", "data": {"types": ret}}
        return {"status": -1, "message": "失败"}

    def get_contacts(self) -> dict:
        """获取完整通讯录"""
        ret = self.wcf.get_contacts()
        if ret:
            return {"status": 0, "message": "成功", "data": {"contacts": ret}}
        return {"status": -1, "message": "失败"}

    def get_friends(self) -> dict:
        """获取好友列表"""
        ret = self.wcf.get_friends()
        if ret:
            return {"status": 0, "message": "成功", "data": {"friends": ret}}
        return {"status": -1, "message": "失败"}

    def get_dbs(self) -> dict:
        """获取所有数据库"""
        ret = self.wcf.get_dbs()
        if ret:
            return {"status": 0, "message": "成功", "data": {"dbs": ret}}
        return {"status": -1, "message": "失败"}

    def get_tables(self, db: str) -> dict:
        """获取 db 中所有表"""
        ret = self.wcf.get_tables(db)
        if ret:
            return {"status": 0, "message": "成功", "data": {"tables": ret}}
        return {"status": -1, "message": "失败"}

    def get_user_info(self) -> dict:
        """获取登录账号个人信息"""
        ret = self.wcf.get_user_info()
        if ret:
            return {"status": 0, "message": "成功", "data": {"ui": ret}}
        return {"status": -1, "message": "失败"}

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

    def query_sql(self,
                  db: str = Body("MicroMsg.db", description="数据库"),
                  sql: str = Body("SELECT * FROM Contact LIMIT 1;", description="SQL 语句")) -> dict:
        """执行 SQL，如果数据量大注意分页，以免 OOM"""
        ret = self.wcf.query_sql(db, sql)
        if ret:
            for row in ret:
                for k, v in row.items():
                    print(k, type(v))
                    if type(v) is bytes:
                        row[k] = base64.b64encode(v)
            return {"status": 0, "message": "成功", "data": {"bs64": ret}}
        return {"status": -1, "message": "失败"}

    def accept_new_friend(self,
                          v3: str = Body("v3", description="加密用户名 (好友申请消息里 v3 开头的字符串)"),
                          v4: str = Body("v4", description="Ticket (好友申请消息里 v4 开头的字符串)"),
                          scene: int = Body(30, description="申请方式 (好友申请消息里的 scene)")) -> dict:
        """通过好友申请"""
        ret = self.wcf.accept_new_friend(v3, v4, scene)
        return {"status": ret, "message": "成功"if ret == 1 else "失败"}

    def add_chatroom_members(self,
                             roomid: str = Body("xxxxxxxx@chatroom", description="待加群的 id"),
                             wxids: str = Body("wxid_xxxxxxxxxxxxx", description="要加到群里的 wxid，多个用逗号分隔")) -> dict:
        """添加群成员"""
        ret = self.wcf.add_chatroom_members(roomid, wxids)
        return {"status": ret, "message": "成功"if ret == 1 else "失败"}

    def receive_transfer(self,
                         wxid: str = Body("wxid_xxxxxxxxxxxxx", description="转账消息里的发送人 wxid"),
                         transferid: str = Body("transferid", description="转账消息里的 transferid")) -> dict:
        """接收转账"""
        ret = self.wcf.receive_transfer(wxid, transferid)
        return {"status": ret, "message": "成功"if ret == 1 else "失败"}

    def decrypt_image(self,
                      src: str = Body("C:\\...", description="加密的图片路径，从图片消息中获取"),
                      dst: str = Body("C:\\...", description="解密的图片路径")) -> dict:
        """接收转账"""
        ret = self.wcf.decrypt_image(src, dst)
        return {"status": ret, "message": "成功"if ret else "失败"}
