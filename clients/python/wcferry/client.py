#! /usr/bin/env python3
# -*- coding: utf-8 -*-

__version__ = "39.2.4.0"

import atexit
import base64
import ctypes
import logging
import mimetypes
import os
import re
import sys
from queue import Queue
from threading import Thread
from time import sleep
from typing import Callable, Dict, List, Optional

import pynng
import requests
from google.protobuf import json_format
from wcferry import wcf_pb2
from wcferry.roomdata_pb2 import RoomData
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
                logerror(f"Exiting... {e}")
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
        block (bool): 是否阻塞等待微信登录，不阻塞的话可以手动获取登录二维码主动登录

    Attributes:
        contacts (list): 联系人缓存，调用 `get_contacts` 后更新
    """

    def __init__(self, host: str = None, port: int = 10086, debug: bool = True, block: bool = True) -> None:
        self._local_mode = False
        self._is_running = False
        self._is_receiving_msg = False
        self._wcf_root = os.path.abspath(os.path.dirname(__file__))
        self._dl_path = f"{self._wcf_root}/.dl"
        os.makedirs(self._dl_path, exist_ok=True)
        self.LOG = logging.getLogger("WCF")
        self.LOG.info(f"wcferry version: {__version__}")
        self.port = port
        self.host = host
        self.sdk = None
        if host is None:
            self._local_mode = True
            self.host = "127.0.0.1"
            self.sdk = ctypes.cdll.LoadLibrary(f"{self._wcf_root}/sdk.dll")
            if self.sdk.WxInitSDK(debug, port) != 0:
                self.LOG.error("初始化失败！")
                os._exit(-1)

        self.cmd_url = f"tcp://{self.host}:{self.port}"

        # 连接 RPC
        self.cmd_socket = pynng.Pair1()  # Client --> Server，发送消息
        self.cmd_socket.send_timeout = 5000  # 发送 5 秒超时
        self.cmd_socket.recv_timeout = 5000  # 接收 5 秒超时
        try:
            self.cmd_socket.dial(self.cmd_url, block=True)
        except Exception as e:
            self.LOG.error(f"连接失败: {e}")
            os._exit(-2)

        self.msg_socket = pynng.Pair1()  # Server --> Client，接收消息
        self.msg_socket.send_timeout = 5000  # 发送 5 秒超时
        self.msg_socket.recv_timeout = 5000  # 接收 5 秒超时
        self.msg_url = self.cmd_url.replace(str(self.port), str(self.port + 1))

        atexit.register(self.cleanup)  # 退出的时候停止消息接收，防止资源占用

        self._is_running = True
        self.contacts = []
        self.msgQ = Queue()
        self._SQL_TYPES = {1: int, 2: float, 3: lambda x: x.decode("utf-8"), 4: bytes, 5: lambda x: None}
        self.self_wxid = ""
        if block:
            self.LOG.info("等待微信登录...")
            while not self.is_login():     # 等待微信登录成功
                sleep(1)
            self.self_wxid = self.get_self_wxid()

    def __del__(self) -> None:
        self.cleanup()

    def cleanup(self) -> None:
        """关闭连接，回收资源"""
        if not self._is_running:
            return

        self.disable_recv_msg()
        self.cmd_socket.close()

        if self._local_mode and self.sdk and self.sdk.WxDestroySDK() != 0:
            self.LOG.error("退出失败！")

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

    def get_qrcode(self) -> str:
        """获取登录二维码，已经登录则返回空字符串"""
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_REFRESH_QRCODE  # FUNC_REFRESH_QRCODE
        rsp = self._send_request(req)

        return rsp.str

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

    def get_msg_types(self) -> Dict:
        """获取所有消息类型"""
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_GET_MSG_TYPES  # FUNC_GET_MSG_TYPES
        rsp = self._send_request(req)
        types = json_format.MessageToDict(rsp.types).get("types", {})
        types = {int(k): v for k, v in types.items()}

        return dict(sorted(dict(types).items()))

    def get_contacts(self) -> List[Dict]:
        """获取完整通讯录"""
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_GET_CONTACTS  # FUNC_GET_CONTACTS
        rsp = self._send_request(req)
        contacts = json_format.MessageToDict(rsp.contacts).get("contacts", [])

        self.contacts.clear()
        for cnt in contacts:
            gender = cnt.get("gender", "")
            if gender == 1:
                gender = "男"
            elif gender == 2:
                gender = "女"
            else:
                gender = ""
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

    def get_tables(self, db: str) -> List[Dict]:
        """获取 db 中所有表

        Args:
            db (str): 数据库名（可通过 `get_dbs` 查询）

        Returns:
            List[Dict]: `db` 下的所有表名及对应建表语句
        """
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_GET_DB_TABLES  # FUNC_GET_DB_TABLES
        req.str = db
        rsp = self._send_request(req)
        tables = json_format.MessageToDict(rsp.tables).get("tables", [])

        return tables

    def get_user_info(self) -> Dict:
        """获取登录账号个人信息"""
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_GET_USER_INFO  # FUNC_GET_USER_INFO
        rsp = self._send_request(req)
        ui = json_format.MessageToDict(rsp.ui)

        return ui

    def get_audio_msg(self, id: int, dir: str, timeout: int = 3) -> str:
        """获取语音消息并转成 MP3
        Args:
            id (int): 语音消息 id
            dir (str): MP3 保存目录（目录不存在会出错）
            timeout (int): 超时时间（秒）

        Returns:
            str: 成功返回存储路径；空字符串为失败，原因见日志。
        """
        def _get_audio_msg(id, dir):
            req = wcf_pb2.Request()
            req.func = wcf_pb2.FUNC_GET_AUDIO_MSG  # FUNC_GET_AUDIO_MSG
            req.am.id = id
            req.am.dir = dir
            rsp = self._send_request(req)

            return rsp.str

        if timeout == 0:
            return _get_audio_msg(id, dir)

        cnt = 0
        while cnt < timeout:
            path = _get_audio_msg(id, dir)
            if path:
                return path
            sleep(1)
            cnt += 1

        self.LOG.error(f"获取超时")
        return ""

    def send_text(self, msg: str, receiver: str, aters: Optional[str] = "") -> int:
        """发送文本消息

        Args:
            msg (str): 要发送的消息，换行使用 `\\\\n` （单杠）；如果 @ 人的话，需要带上跟 `aters` 里数量相同的 @
            receiver (str): 消息接收人，wxid 或者 roomid
            aters (str): 要 @ 的 wxid，多个用逗号分隔；`@所有人` 只需要 `notify@all`

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

    def _download_file(self, url: str) -> str:
        path = None
        if not self._local_mode:
            self.LOG.error(f"只有本地模式才支持网络路径！")
            return path

        try:
            headers = {
                'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/100.0.4896.127 Safari/537.36', }
            rsp = requests.get(url, headers=headers, stream=True, timeout=60)
            rsp.raw.decode_content = True

            # 提取文件名
            fname = os.path.basename(url)
            ct = rsp.headers["content-type"]
            ext = mimetypes.guess_extension(ct)
            if ext:
                if ext not in fname:
                    fname = fname + ext
                else:
                    fname = fname.split(ext)[0] + ext

            # 保存文件，用完后删除
            with open(f"{self._dl_path}/{fname}", "wb") as of:
                of.write(rsp.content)

            path = os.path.normpath(f"{self._dl_path}/{fname}")
        except Exception as e:
            self.LOG.error(f"网络资源下载失败: {e}")

        return path

    def _process_path(self, path) -> str:
        """处理路径，如果是网络路径则下载文件
        """
        if path.startswith("http"):
            path = self._download_file(path)
            if not path:
                return -102  # 下载失败
        elif not os.path.exists(path):
            self.LOG.error(f"图片或者文件不存在，请检查路径: {path}")
            return -101  # 文件不存在

        return path

    def send_image(self, path: str, receiver: str) -> int:
        """发送图片，非线程安全

        Args:
            path (str): 图片路径，如：`C:/Projs/WeChatRobot/TEQuant.jpeg` 或 `https://raw.githubusercontent.com/lich0821/WeChatFerry/master/assets/TEQuant.jpg`
            receiver (str): 消息接收人，wxid 或者 roomid

        Returns:
            int: 0 为成功，其他失败
        """
        path = self._process_path(path)
        if isinstance(path, int):
            return path

        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_SEND_IMG  # FUNC_SEND_IMG
        req.file.path = path
        req.file.receiver = receiver
        rsp = self._send_request(req)
        return rsp.status

    def send_file(self, path: str, receiver: str) -> int:
        """发送文件，非线程安全

        Args:
            path (str): 本地文件路径，如：`C:/Projs/WeChatRobot/README.MD` 或 `https://raw.githubusercontent.com/lich0821/WeChatFerry/master/README.MD`
            receiver (str): 消息接收人，wxid 或者 roomid

        Returns:
            int: 0 为成功，其他失败
        """
        path = self._process_path(path)
        if isinstance(path, int):
            return path

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
        raise Exception("Not implemented, yet")
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

    def send_rich_text(
            self, name: str, account: str, title: str, digest: str, url: str, thumburl: str, receiver: str) -> int:
        """发送富文本消息
        卡片样式：
            |-------------------------------------|
            |title, 最长两行
            |(长标题, 标题短的话这行没有)
            |digest, 最多三行，会占位    |--------|
            |digest, 最多三行，会占位    |thumburl|
            |digest, 最多三行，会占位    |--------|
            |(account logo) name
            |-------------------------------------|
        Args:
            name (str): 左下显示的名字
            account (str): 填公众号 id 可以显示对应的头像（gh_ 开头的）
            title (str): 标题，最多两行
            digest (str): 摘要，三行
            url (str): 点击后跳转的链接
            thumburl (str): 缩略图的链接
            receiver (str): 接收人, wxid 或者 roomid

        Returns:
            int: 0 为成功，其他失败
        """
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_SEND_RICH_TXT  # FUNC_SEND_RICH_TXT
        req.rt.name = name
        req.rt.account = account
        req.rt.title = title
        req.rt.digest = digest
        req.rt.url = url
        req.rt.thumburl = thumburl
        req.rt.receiver = receiver

        rsp = self._send_request(req)
        return rsp.status

    def send_pat_msg(self, roomid: str, wxid: str) -> int:
        """拍一拍群友

        Args:
            roomid (str): 群 id
            wxid (str): 要拍的群友的 wxid

        Returns:
            int: 1 为成功，其他失败
        """
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_SEND_PAT_MSG  # FUNC_SEND_PAT_MSG
        req.pm.roomid = roomid
        req.pm.wxid = wxid

        rsp = self._send_request(req)
        return rsp.status

    def forward_msg(self, id: int, receiver: str) -> int:
        """转发消息。可以转发文本、图片、表情、甚至各种 XML；
        语音也行，不过效果嘛，自己验证吧。

        Args:
            id (str): 待转发消息的 id
            receiver (str): 消息接收者，wxid 或者 roomid

        Returns:
            int: 1 为成功，其他失败
        """
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_FORWARD_MSG  # FUNC_FORWARD_MSG
        req.fm.id = id
        req.fm.receiver = receiver

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

    def enable_receiving_msg(self, pyq=False) -> bool:
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
        req.flag = pyq
        rsp = self._send_request(req)
        if rsp.status != 0:
            return False

        self._is_receiving_msg = True
        # 阻塞，把控制权交给用户
        # self.listening_msg(callback)

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

    def query_sql(self, db: str, sql: str) -> List[Dict]:
        """执行 SQL，如果数据量大注意分页，以免 OOM

        Args:
            db (str): 要查询的数据库
            sql (str): 要执行的 SQL

        Returns:
            List[Dict]: 查询结果
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

    def get_friends(self) -> List[Dict]:
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
            if (cnt["wxid"].endswith("@chatroom") or    # 群聊
                    cnt["wxid"].startswith("gh_") or    # 公众号
                    cnt["wxid"] in not_friends.keys()   # 其他杂号
                ):
                continue
            friends.append(cnt)

        return friends

    def receive_transfer(self, wxid: str, transferid: str, transactionid: str) -> int:
        """接收转账

        Args:
            wxid (str): 转账消息里的发送人 wxid
            transferid (str): 转账消息里的 transferid
            transactionid (str): 转账消息里的 transactionid

        Returns:
            int: 1 为成功，其他失败
        """
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_RECV_TRANSFER  # FUNC_RECV_TRANSFER
        req.tf.wxid = wxid
        req.tf.tfid = transferid
        req.tf.taid = transactionid
        rsp = self._send_request(req)
        return rsp.status

    def refresh_pyq(self, id: int = 0) -> int:
        """刷新朋友圈

        Args:
            id (int): 开始 id，0 为最新页

        Returns:
            int: 1 为成功，其他失败
        """
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_REFRESH_PYQ  # FUNC_REFRESH_PYQ
        req.ui64 = id
        rsp = self._send_request(req)
        return rsp.status

    def download_attach(self, id: int, thumb: str, extra: str) -> int:
        """下载附件（图片、视频、文件）。这方法别直接调用，下载图片使用 `download_image`。

        Args:
            id (int): 消息中 id
            thumb (str): 消息中的 thumb
            extra (str): 消息中的 extra

        Returns:
            int: 0 为成功, 其他失败。
        """
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_DOWNLOAD_ATTACH  # FUNC_DOWNLOAD_ATTACH
        req.att.id = id
        req.att.thumb = thumb
        req.att.extra = extra
        rsp = self._send_request(req)
        return rsp.status

    def get_info_by_wxid(self, wxid: str) -> dict:
        """通过 wxid 查询微信号昵称等信息

        Args:
            wxid (str): 联系人 wxid

        Returns:
            dict: {wxid, code, name, gender}
        """
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_GET_CONTACT_INFO  # FUNC_GET_CONTACT_INFO
        req.str = wxid
        rsp = self._send_request(req)
        contacts = json_format.MessageToDict(rsp.contacts).get("contacts", [])

        contact = {}
        for cnt in contacts:
            gender = cnt.get("gender", "")
            if gender == 1:
                gender = "男"
            elif gender == 2:
                gender = "女"
            else:
                gender = ""
            contact = {
                "wxid": cnt.get("wxid", ""),
                "code": cnt.get("code", ""),
                "remark": cnt.get("remark", ""),
                "name": cnt.get("name", ""),
                "country": cnt.get("country", ""),
                "province": cnt.get("province", ""),
                "city": cnt.get("city", ""),
                "gender": gender}

        return contact

    def revoke_msg(self, id: int = 0) -> int:
        """撤回消息

        Args:
            id (int): 待撤回消息的 id

        Returns:
            int: 1 为成功，其他失败
        """
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_REVOKE_MSG  # FUNC_REVOKE_MSG
        req.ui64 = id
        rsp = self._send_request(req)
        return rsp.status

    def decrypt_image(self, src: str, dir: str) -> str:
        """解密图片。这方法别直接调用，下载图片使用 `download_image`。

        Args:
            src (str): 加密的图片路径
            dir (str): 保存图片的目录

        Returns:
            str: 解密图片的保存路径
        """
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_DECRYPT_IMAGE  # FUNC_DECRYPT_IMAGE
        req.dec.src = src
        req.dec.dst = dir
        rsp = self._send_request(req)
        return rsp.str

    def get_ocr_result(self, extra: str, timeout: int = 2) -> str:
        """获取 OCR 结果。鸡肋，需要图片能自动下载；通过下载接口下载的图片无法识别。

        Args:
            extra (str): 待识别的图片路径，消息里的 extra

        Returns:
            str: OCR 结果
        """
        def _inner(extra):
            req = wcf_pb2.Request()
            req.func = wcf_pb2.FUNC_EXEC_OCR  # FUNC_EXEC_OCR
            req.str = extra
            rsp = self._send_request(req)
            ocr = json_format.MessageToDict(rsp.ocr)
            return ocr.get("status", 0), ocr.get("result", "")

        cnt = 0
        while True:
            status, result = _inner(extra)
            if status == 0:
                break

            cnt += 1
            if cnt > timeout:
                break

            sleep(1)

        if status != 0:
            self.LOG.error(f"OCR failed, status: {status}")

        return result

    def download_image(self, id: int, extra: str, dir: str, timeout: int = 30) -> str:
        """下载图片

        Args:
            id (int): 消息中 id
            extra (str): 消息中的 extra
            dir (str): 存放图片的目录（目录不存在会出错）
            timeout (int): 超时时间（秒）

        Returns:
            str: 成功返回存储路径；空字符串为失败，原因见日志。
        """
        if self.download_attach(id, "", extra) != 0:
            self.LOG.error(f"下载失败")
            return ""
        cnt = 0
        while cnt < timeout:
            path = self.decrypt_image(extra, dir)
            if path:
                return path
            sleep(1)
            cnt += 1

        self.LOG.error(f"下载超时")
        return ""

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

    def del_chatroom_members(self, roomid: str, wxids: str) -> int:
        """删除群成员

        Args:
            roomid (str): 群的 id
            wxids (str): 要删除成员的 wxid，多个用逗号分隔

        Returns:
            int: 1 为成功，其他失败
        """
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_DEL_ROOM_MEMBERS  # FUNC_DEL_ROOM_MEMBERS
        req.m.roomid = roomid
        req.m.wxids = wxids.replace(" ", "")
        rsp = self._send_request(req)
        return rsp.status

    def invite_chatroom_members(self, roomid: str, wxids: str) -> int:
        """邀请群成员

        Args:
            roomid (str): 群的 id
            wxids (str): 要邀请成员的 wxid, 多个用逗号`,`分隔

        Returns:
            int: 1 为成功，其他失败
        """
        req = wcf_pb2.Request()
        req.func = wcf_pb2.FUNC_INV_ROOM_MEMBERS  # FUNC_INV_ROOM_MEMBERS
        req.m.roomid = roomid
        req.m.wxids = wxids.replace(" ", "")
        rsp = self._send_request(req)
        return rsp.status

    def get_chatroom_members(self, roomid: str) -> Dict:
        """获取群成员

        Args:
            roomid (str): 群的 id

        Returns:
            Dict: 群成员列表: {wxid1: 昵称1, wxid2: 昵称2, ...}
        """
        members = {}
        contacts = self.query_sql("MicroMsg.db", "SELECT UserName, NickName FROM Contact;")
        contacts = {contact["UserName"]: contact["NickName"]for contact in contacts}
        crs = self.query_sql("MicroMsg.db", f"SELECT RoomData FROM ChatRoom WHERE ChatRoomName = '{roomid}';")
        if not crs:
            return members

        bs = crs[0].get("RoomData")
        if not bs:
            return members

        crd = RoomData()
        crd.ParseFromString(bs)
        if not bs:
            return members

        for member in crd.members:
            members[member.wxid] = member.name if member.name else contacts.get(member.wxid, "")

        return members

    def get_alias_in_chatroom(self, wxid: str, roomid: str) -> str:
        """获取群名片

        Args:
            wxid (str): wxid
            roomid (str): 群的 id

        Returns:
            str: 群名片
        """
        nickname = self.query_sql("MicroMsg.db", f"SELECT NickName FROM Contact WHERE UserName = '{wxid}';")
        if not nickname:
            return ""

        nickname = nickname[0].get("NickName", "")

        crs = self.query_sql("MicroMsg.db", f"SELECT RoomData FROM ChatRoom WHERE ChatRoomName = '{roomid}';")
        if not crs:
            return ""

        bs = crs[0].get("RoomData")
        if not bs:
            return ""

        crd = RoomData()
        crd.ParseFromString(bs)
        for member in crd.members:
            if member.wxid == wxid:
                return member.name if member.name else nickname

        return ""
