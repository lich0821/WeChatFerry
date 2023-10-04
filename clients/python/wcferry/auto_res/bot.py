# -*- coding: utf-8 -*-

from typing import Callable, Any
from ..event import Event
from abc import abstractmethod
from ..client import Wcf
import logging


class Register(Event):
    def __init__(self, debug=True, **kwargs):
        super(Register, self).__init__()
        logging.basicConfig(level='DEBUG', format="%(asctime)s %(message)s")
        self._LOG = logging.getLogger("Demo")
        self._LOG.info("开始初始化...")
        # 默认连接本地服务
        self._wcf = Wcf(debug=debug, **kwargs)

    @abstractmethod
    def _process_msg(self, wcf: Wcf):
        """
        有消息的时候，通知分发器分发消息
        :param wcf: Wcf
        :return: None
        """
        raise NotImplementedError

    @abstractmethod
    def _register(self,
                  func: Callable[[Any], Any]):
        """
        消息处理工厂, 所有消息处理函数都会汇总在这里提交给事件分发器
        :param func: 被装饰的待处理消息函数
        :return: func
        """
        raise NotImplementedError

    @abstractmethod
    def _processing_async_func(self,
                               isGroup: bool,
                               isDivision: bool,
                               isPyq: bool):
        """
        异步函数消息处理函数, 用来接受非协程函数
        参数:
        :param isGroup 对消息进行限制, 当为True时, 只接受群消息, 当为False时, 只接受私聊消息,
                       注意! 仅当isDivision为 True时, isGroup参数生效
        :param isPyq 是否接受朋友圈消息
        :param isDivision 是否对消息分组
        """
        raise NotImplementedError

    @abstractmethod
    def _processing_universal_func(self,
                                   isGroup: bool,
                                   isDivision: bool,
                                   isPyq: bool):
        """
        同步函数消息处理函数, 用来接受非协程函数
        参数:
        :param isGroup 对消息进行限制, 当为True时, 只接受群消息, 当为False时, 只接受私聊消息,
                       注意! 仅当isDivision为 True时, isGroup参数生效
        :param isPyq 是否接受朋友圈消息
        :param isDivision 是否对消息分组
        """
        raise NotImplementedError

    @abstractmethod
    def message_register(self,
                         isGroup: bool = False,
                         isDivision: bool = False,
                         isPyq: bool = False):
        """
        外部可访问接口
        消息处理函数注册器, 用来接受同步函数
        参数:
        :param isGroup 对消息进行限制, 当为True时, 只接受群消息, 当为False时, 只接受私聊消息,
                       注意! 仅当isDivision为 True时, isGroup参数生效
        :param isPyq 是否接受朋友圈消息
        :param isDivision 是否对消息分组
        """
        raise NotImplementedError

    @abstractmethod
    def async_message_register(self,
                               isGroup: bool = False,
                               isDivision: bool = False,
                               isPyq: bool = False):
        """
        外部可访问接口
        消息处理函数注册器, 用来接受异步函数
        参数:
        :param isGroup 对消息进行限制, 当为True时, 只接受群消息, 当为False时, 只接受私聊消息,
                       注意! 仅当isDivision为 True时, isGroup参数生效
        :param isPyq 是否接受朋友圈消息
        :param isDivision 是否对消息分组
        """
        raise NotImplementedError

    @abstractmethod
    def run(self, *args, **kwargs):
        """
        启动程序, 开始接受消息
        """
        raise NotImplementedError

    @abstractmethod
    def stop_receiving(self):
        """
        停止接受消息
        """
        raise NotImplementedError
