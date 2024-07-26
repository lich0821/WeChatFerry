# -*- coding: utf-8 -*-

import re
from datetime import datetime

from wcferry import wcf_pb2


class WxMsg():
    """微信消息

    Attributes:
        type (int): 消息类型，可通过 `get_msg_types` 获取
        id (str): 消息 id
        xml (str): 消息 xml 部分
        sender (str): 消息发送人
        roomid (str): （仅群消息有）群 id
        content (str): 消息内容
        thumb (str): 视频或图片消息的缩略图路径
        extra (str): 视频或图片消息的路径
    """

    def __init__(self, msg: wcf_pb2.WxMsg) -> None:
        self._is_self = msg.is_self
        self._is_group = msg.is_group
        self.type = msg.type
        self.id = msg.id
        self.ts = msg.ts
        self.sign = msg.sign
        self.xml = msg.xml
        self.sender = msg.sender
        self.roomid = msg.roomid
        self.content = msg.content
        self.thumb = msg.thumb
        self.extra = msg.extra

    def __str__(self) -> str:
        s = f"{'自己发的:' if self._is_self else ''}"
        s += f"{self.sender}[{self.roomid}]|{self.id}|{datetime.fromtimestamp(self.ts)}|{self.type}|{self.sign}"
        s += f"\n{self.xml.replace(chr(10), '').replace(chr(9),'')}\n"
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
        """是否被 @：群消息，在 @ 名单里，并且不是 @ 所有人"""
        if not self.from_group():
            return False  # 只有群消息才能 @

        if not re.findall(f"<atuserlist>[\s|\S]*({wxid})[\s|\S]*</atuserlist>", self.xml):
            return False  # 不在 @ 清单里

        if re.findall(r"@(?:所有人|all|All)", self.content):
            return False  # 排除 @ 所有人

        return True

    def is_text(self) -> bool:
        """是否文本消息"""
        return self.type == 1
