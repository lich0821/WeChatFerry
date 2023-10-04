# coding: utf-8
# @Author: 小杨大帅哥
import logging
from abc import abstractmethod
import asyncio
import logging


class Event(object):
    _message_callback_func = {}
    _message_callback_func_list = []
    _loop = asyncio.get_event_loop()

    def __init__(self):
        super(Event, self).__init__()
        self._message = None
        self._logger: logging = logging.getLogger()

    @abstractmethod
    def _add_callback(self, func, *args, **kwargs):
        """
        消息处理函数加载器
        :param func: 消息处理函数
        :param args: 消息处理函数参数
        :param kwargs: 消息处理函数参数
        """
        raise NotImplementedError

    @abstractmethod
    def _run_func(self):
        """
        消息分发器, 将消息发送给所有消息处理函数
        """
        raise NotImplementedError
