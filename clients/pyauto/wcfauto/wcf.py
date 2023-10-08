# -*- coding: utf-8 -*-

import re
import time
from wcfauto.msg_list import msg_list
from wcferry import Wcf, WxMsg


class WcfV2(Wcf):
    def __init__(self, host: str = None, port: int = 10086, debug: bool = True) -> None:
        super().__init__(host, port, debug)

    def get_msg(self, block=True) -> WxMsg:
        """从消息队列中获取消息

        Args:
            block (bool): 是否阻塞，默认阻塞

        Returns:
            WxMsg: 微信消息

        Raises:
            Empty: 如果阻塞并且超时，抛出空异常，需要用户自行捕获
        """
        return WxMsgV2(self.msgQ.get(block, timeout=1))


class WxMsgV2(WxMsg):
    """微信消息

    Attributes:
        type (int): 消息类型，可通过 `wcf类的 get_msg_types` 获取
        id (str): 消息 id
        xml (str): 消息 xml 部分
        sender (str): 消息发送人
        roomid (str): （仅群消息有）群 id
        content (str): 消息内容
        thumb (str): 视频或图片消息的缩略图路径
        extra (str): 视频或图片消息的路径
    """

    def __init__(self, msg: WxMsg) -> None:
        self._is_group = msg._is_group
        self._is_self = msg._is_self
        self._type = msg.type
        self._id = msg.id
        self._ts = msg.ts
        self._sign = msg.sign
        self._xml = msg.xml
        self._sender = msg.sender
        self._roomid = msg.roomid
        self._content = msg.content
        self._thumb = msg.thumb.replace("\\", "/")
        self._extra = msg.extra.replace("\\", "/")
        self.__data = {'isSelf': True if self._is_self else False,
                       'isGroup': True if self._is_group else False,
                       'isPyq': True if self._type == 0 else False,
                       'data': {
                           'type': self._type,
                           'content': self._content,
                           'sender': self._sender,
                           'msgid': self._id,
                           'roomid': self._roomid if self._roomid else None,
                           'xml': self._xml,
                           'thumb': self._thumb if self._thumb else None,
                           'extra': self._extra if self._extra else None,
                           'time': int(time.time() * 1000),
                       }, 'revokmsgid': None, 'isRevokeMsg': False}
        self.__revokmsg_p()
        self.__initial()
        msg_list.append(self)

    def __revokmsg_p(self):
        rmsg = self.__data['data']['content']
        rev_type = re.findall(r'<sysmsg type="(.*?)"\s?', rmsg)
        rev_w = re.findall(r"<replacemsg><!\[CDATA\[(.*?)]]></replacemsg>", rmsg)
        if len(rev_type) == 0 or len(rev_w) == 0:
            return
        if rev_type[0] == 'revokemsg' and '撤回了一条消息' in rev_w[0]:
            self.__data['data']['content'] = rev_w[0]
            self.__data['isRevokeMsg'] = True
            self.__data['revokmsgid'] = re.findall('<newmsgid>(.*?)</newmsgid>', rmsg)[0]

    def __initial(self):
        try:
            if self.__data['data']['type'] == 51:
                op_id = re.findall(r"<op id='(\d+)'>", self.__data['data']['content'])[0]
                name = re.findall(r'<name>(.*?)</name>', self.__data['data']['content'])[0].strip()
                if name == 'lastMessage' and str(op_id) == '2':
                    username = re.findall(r'<username>(.*?)</username>', self.__data['data']['content'])[0]
                    self.__data['data']['xml'] = self.__data['data']['content']
                    self.__data['data']['content'] = f'其他设备进入 [{username}] 聊天界面'
                    return
                if name == 'HandOffMaster' and str(op_id) == '11':
                    opcode = re.findall(r'opcode="(\d+)"', self.__data['data']['content'])[0]
                    self.__data['data']['xml'] = self.__data['data']['content']
                    title = '' or re.findall(r'<title><!\[CDATA\[(.*?)]]></title>', self.__data['data']['content'])[0]
                    type_id = re.findall(r'<handoff\s+type="(\d+)"', self.__data['data']['content'])[0]
                    type_map = {'2': '公众号文章', '3': '小程序'}
                    if str(opcode) == '1':
                        self.__data['data']['content'] = f'其他设备进入{type_map[str(type_id)]} [{title}]'
                    elif str(opcode) == '2':
                        self.__data['data']['content'] = f'其他设备离开{type_map[str(type_id)]} [{title}]'
                    elif str(opcode) == '3':
                        self.__data['data']['content'] = f'其他设备在{type_map[str(type_id)]} [{title}] 里进行跳转'
                    elif str(opcode) == '4':
                        self.__data['data']['content'] = f'其他设备进入{type_map[str(type_id)]} [{title}]'
                        return
                if name == 'MomentsUnreadMsgStatus' or name == 'MomentsTimelineStatus':
                    self.__data['data']['xml'] = self.__data['data']['content']
                    self.__data['data']['content'] = f'其他设备进入朋友圈'
                    return
        except:
            pass

    def __str__(self) -> str:
        return repr(self.__data)

    def __repr__(self) -> str:
        return repr(self.__data)

    def __getitem__(self, key):
        return self.__data[key]

    def __getattr__(self, item):
        if item in ['content', 'sender', 'roomid', 'xml', 'thumb', 'extra', 'type']:
            return self.__data['data'][item]
        if item == 'id':
            return self.__data['data']['msgid']
        if item == 'ts':
            return self._ts
        if item == 'sign':
            return self._sign

    def __setitem__(self, key, value):
        self.__data[key] = value

    def is_image(self) -> bool:
        """是否是图片"""
        return self.type == 3 and ('imgdatahash' in self.__data['data']['content'])

    def is_voice(self) -> bool:
        """是否是语音"""
        return self.type == 34 and ('voicemsg' in self.__data['data']['content'])

    def is_video(self) -> bool:
        """是否是视频"""
        return self.type == 43 and ('videomsg' in self.__data['data']['content'])

    def is_pyq(self) -> bool:
        return self.type == 0

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

        if not re.findall(f"<atuserlist>.*({wxid}).*</atuserlist>", self.xml):
            return False  # 不在 @ 清单里

        if re.findall(r"@(?:所有人|all|All)", self.content):
            return False  # 排除 @ 所有人

        return True

    def is_text(self) -> bool:
        """是否文本消息"""
        return self.type == 1

    def get_revoke_msg(self) -> "WxMsg" or None:
        """
        获取撤回的消息
        暂不支持图片、视频、文件等有关文件的消息
        注意仅有撤回的消息在调用函数会返回消息内容, 自行修改 msg内容或者不是撤回的消息在调用该函数后会返回 None
        未撤回的消息调用该函数会返回 None
        :return: 返回被撤回的信息, 信息的类仍然是 WxMsg
        """
        if self.__data['isRevokeMsg'] and (self.__data['revokmsgid'] is not None):
            return msg_list.find_msg(self.__data['revokmsgid'])
        return None
