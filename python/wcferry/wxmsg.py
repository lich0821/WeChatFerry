# -*- coding: utf-8 -*-

from wcferry import wcf_pb2

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
        """是否被 @：群消息，在 @ 名单里，并且不是 @ 所有人"""
        return self.from_group() and re.findall(
            f"<atuserlist>.*({wxid}).*</atuserlist>", self.xml) and not re.findall(r"@(?:所有人|all)", self.xml)

    def is_text(self) -> bool:
        """是否文本消息"""
        return self.type == 1
