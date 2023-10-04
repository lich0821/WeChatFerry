# coding: utf-8
# @Author: 小杨大帅哥
import asyncio
import functools
import logging
import queue
import traceback
from threading import Thread
from typing import Callable, Any
from ..client import Wcf
from ..wxmsg import WxMsg


def load_function(cls):
    cls._process_msg = _process_msg
    cls._register = _register
    cls._processing_async_func = _processing_async_func
    cls._processing_universal_func = _processing_universal_func
    cls.message_register = message_register
    cls.async_message_register = async_message_register
    cls.run = run
    cls.stop_receiving = stop_receiving
    return cls


def _process_msg(self, wcf: Wcf):
    """有消息的时候，通知分发器分发消息"""
    while wcf.is_receiving_msg():
        try:
            msg = wcf.get_msg()
            self._message = msg
            self._run_func()
        except queue.Empty:
            pass


def _register(self,
              func: Callable[[Any], Any]):
    self._add_callback(func, self._wcf)
    # 此处必须返回被装饰函数原函数, 否则丢失被装饰函数信息
    return func


def _processing_async_func(self,
                           isGroup: bool,
                           isDivision: bool,
                           isPyq: bool,):
    def _async_func(func):

        @functools.wraps(func)
        @self._register
        async def __async_func(bot: Wcf, message: WxMsg):
            try:
                # 判断被装饰函数是否为协程函数, 本函数要求是协程函数
                if not asyncio.iscoroutinefunction(func): raise ValueError(
                    f'这里应使用协程函数, 而被装饰函数-> ({func.__name__}) <-是非协程函数')
                if message.is_pyq() and isPyq:
                    return await func(bot, message)
                if not isDivision:
                    return await func(bot, message)
                if message.from_group() and isGroup:
                    return await func(bot, message)
                if not message.from_group() and not isGroup:
                    return await func(bot, message)
            except:
                traceback.print_exc()
        return __async_func
    return _async_func


def _processing_universal_func(self,
                               isGroup: bool,
                               isDivision: bool,
                               isPyq: bool, ):
    def _universal_func(func):

        @functools.wraps(func)
        @self._register
        def universal_func(bot: Wcf, message: WxMsg):
            try:
                # 判断被装饰函数是否为协程函数, 本函数要求是协程函数
                if asyncio.iscoroutinefunction(func): raise ValueError(
                    f'这里应使用非协程函数, 而被装饰函数-> ({func.__name__}) <-协程函数')
                if message.is_pyq() and isPyq:
                    return func(bot, message)
                if not isDivision:
                    return func(bot, message)
                if message.from_group() and isGroup:
                    return func(bot, message)
                if not message.from_group() and not isGroup:
                    return func(bot, message)
            except:
                traceback.print_exc()
            return None
        return universal_func
    return _universal_func


def message_register(self,
                     isGroup: bool = False,
                     isDivision: bool = False,
                     isPyq: bool = False):
    return self._processing_universal_func(isGroup, isDivision, isPyq)


def async_message_register(self,
                           isGroup: bool = False,
                           isDivision: bool = False,
                           isPyq: bool = False):
    return self._processing_async_func(isGroup, isDivision, isPyq)


def run(self, *args, **kwargs):
    self._wcf.enable_receiving_msg(*args, pyq=True, **kwargs)
    Thread(target=self._process_msg, name="GetMessage", args=(self._wcf,), daemon=True).start()
    self._LOG.debug("开始接受消息")
    self._wcf.keep_running()

def stop_receiving(self):
    return self._wcf.disable_recv_msg()
