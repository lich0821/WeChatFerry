# -*- coding: utf-8 -*-

from typing import Callable, Any
from wcfauto.event import Event
from abc import abstractmethod
from wcfauto.wcf import WcfV2 as Wcf, WxMsgV2 as WxMsg
import logging


class Register(Event):
    """
    消息注册器
    """
    def __init__(self,
                 debug=True,
                 **kwargs):
        super(Register, self).__init__()
        logging.basicConfig(level='DEBUG', format="%(asctime)s %(message)s")
        self._LOG = logging.getLogger("Demo")
        self._LOG.info("开始初始化...")
        # 默认连接本地服务
        self._wcf = Wcf(debug=debug, **kwargs)

    @abstractmethod
    def _process_msg(self,
                     wcf: Wcf):
        """
        外部不可访问接口
        有消息的时候，通知分发器分发消息
        :param wcf: Wcf
        :return: None
        """
        raise NotImplementedError

    @abstractmethod
    def _register(self,
                  kind: str,
                  register_name: str,
                  allow_other_receive: bool,
                  judge_msg: Callable[[Any], Any]):
        """
        外部不可访问接口
        消息处理工厂, 所有消息处理函数都会汇总在这里提交给事件分发器
        :param kind: 用于区分被装饰函数属于异步或者同步函数, 主要用于消息分发函数中做区分主要两大类函数
        :param register_name: 函数类名 (用于一个装饰器用于装饰多个函数时, 将这些函数统一取名一个类名)
        :param allow_other_receive 是否允许其他消息函数(指函数类名不同且必须同为同一个 kind的消息函数(即同为异步或者同步))接收消息
        :param judge_msg 用来判断消息是否是本函数要求的消息
        """
        raise NotImplementedError

    @abstractmethod
    def _processing_async_func(self,
                               isGroup: bool,
                               isDivision: bool,
                               isPyq: bool,
                               register_name: str,
                               allow_other_receive: bool,
                               judge_msg: Callable[[Any], bool]):
        """
        外部不可访问接口
        异步函数消息处理函数, 用来接受协程函数, 此函数为异步装饰器函数基函数
        参数:
        :param isGroup 对消息进行限制, 当为True时, 只接受群消息, 当为False时, 只接受私聊消息,
                       注意! 仅当isDivision为 True时, isGroup参数生效
        :param isPyq 是否接受朋友圈消息
        :param isDivision 是否对消息分组
        :param register_name 函数类名 (用于一个装饰器用于装饰多个函数时, 将这些函数统一取名一个类名)
        :param allow_other_receive 是否允许其他消息函数(指函数类名不同且必须同为同一个 kind的消息函数(即同为异步或者同步))接收消息
        :param judge_msg 用来判断消息是否是本函数要求的消息
        """
        raise NotImplementedError

    @abstractmethod
    def _processing_universal_func(self,
                                   isGroup: bool,
                                   isDivision: bool,
                                   isPyq: bool,
                                   register_name: str,
                                   allow_other_receive: bool,
                                   judge_msg: Callable[[Any], bool]):
        """
        外部不可访问接口
        同步函数消息处理函数, 用来接受非协程函数, 此函数为同步装饰器函数基函数
        参数:
        :param isGroup 对消息进行限制, 当为True时, 只接受群消息, 当为False时, 只接受私聊消息,
                       注意! 仅当isDivision为 True时, isGroup参数生效
        :param isPyq 是否接受朋友圈消息
        :param isDivision 是否对消息分组
        :param register_name 函数类名 (用于一个装饰器用于装饰多个函数时, 将这些函数统一取名一个类名)
        :param allow_other_receive 是否允许其他消息函数(指函数类名不同且必须同为同一个 kind的消息函数(即同为异步或者同步))接收消息
        :param judge_msg 用来判断消息是否是本函数要求的消息
        """
        raise NotImplementedError

    @abstractmethod
    def message_register(self,
                         isGroup: bool = False,
                         isDivision: bool = False,
                         isPyq: bool = False):
        """
        外部可访问接口
        [所有消息] 处理函数注册器, 用来接受同步函数
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
        [所有消息] 处理函数注册器, 用来接受异步函数
        参数:
        :param isGroup 对消息进行限制, 当为True时, 只接受群消息, 当为False时, 只接受私聊消息,
                       注意! 仅当isDivision为 True时, isGroup参数生效
        :param isPyq 是否接受朋友圈消息
        :param isDivision 是否对消息分组
        """
        raise NotImplementedError

    @abstractmethod
    def revoke_message_register(self,
                                isGroup: bool = False,
                                isDivision: bool = False,
                                isPyq: bool = False,
                                allow_other_receive: bool = True):
        """
        外部可访问接口
        [撤回消息] 处理函数注册器, 用来接受异步函数
        参数:
        :param isGroup 对消息进行限制, 当为True时, 只接受群消息, 当为False时, 只接受私聊消息,
                       注意! 仅当isDivision为 True时, isGroup参数生效
        :param isPyq 是否接受朋友圈消息
        :param isDivision 是否对消息分组
        :param allow_other_receive 是否允许符合本函数要求的消息(撤回消息)被其他消息函数(指函数类名不同且
                                    必须同为同一个 kind的消息函数(即同为异步或者同步))接受
        """
        raise NotImplementedError

    def group_changed_register(self,
                               isPyq: bool = False,
                               allow_other_receive: bool = True):
        """
        外部可访问接口
        [群成员变动消息]处理函数注册器, 用来接受异步函数
        参数:
        :param isGroup 对消息进行限制, 当为True时, 只接受群消息, 当为False时, 只接受私聊消息,
                       注意! 仅当isDivision为 True时, isGroup参数生效
        :param isPyq 是否接受朋友圈消息
        :param isDivision 是否对消息分组
        :param allow_other_receive 是否允许符合本函数要求的消息(群成员消息)被其他消息函数(指函数类名不同且
                                    必须同为同一个 kind的消息函数(即同为异步或者同步))接受
        """
        raise NotImplementedError

    def custom_message_register(self,
                                register_name: str,
                                msg_judge_func: Callable[[WxMsg], bool],
                                allow_other_receive: bool,
                                isGroup: bool = False,
                                isDivision: bool = False,
                                isPyq: bool = False):
        """
        外部可访问接口
        [自定义消息]处理函数注册器, 用来接受异步函数
        参数:
        :param isGroup 对消息进行限制, 当为True时, 只接受群消息, 当为False时, 只接受私聊消息,
                       注意! 仅当isDivision为 True时, isGroup参数生效
        :param isPyq 是否接受朋友圈消息
        :param isDivision 是否对消息分组
        :param register_name 注册函数类名, 注意不可为空, 注意函数类名决定最后消息分发结果, 因此每个新函数的类名应是不同的
        :param msg_judge_func 判断消息是否为符合自定义消息的函数 (函数直接受一个参数 WcfMsg作为参数, 返回值为 bool)
                                    在函数内编写需要符合自定义要求的函数, 但函数返回值必须为 bool类型, 符合要求返回 True,
                                    不符合要求返回 False
        :param allow_other_receive 是否允许符合本函数要求的消息(自定义符合消息)被其他消息函数(指函数类名不同且
                                    必须同为同一个 kind的消息函数(即同为异步或者同步))接受
        """
        raise NotImplementedError

    @abstractmethod
    def run(self, pyq=True):
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
