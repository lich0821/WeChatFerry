# -*- coding: utf-8 -*-

import traceback
from typing import Callable, Any
from wcfauto.wcf import WcfV2 as Wcf
from wcfauto.wcf import WxMsgV2 as WxMsg
import asyncio
from threading import Thread


def load_function(cls):
    cls._add_callback = _add_callback
    cls._run_func = _run_func
    return cls


def _add_callback(self,
                  func: Callable[[Any], Any],
                  bot: Wcf,
                  kind: str,
                  register_name: str,
                  allow_other_rec: bool,
                  judge_msg: Callable[[WxMsg], bool]):
    """
    消息处理函数加载器
    :param func: 装饰器装饰的函数
    :param bot: Wcf类
    :param kind: 装饰器所处类别, 分为异步和同步
    :param register_name: 装饰器所处类别下的函数类名(主要区分不同装饰器的作用, 以及为其中的 allow_other_rec参数做准备)
    :param allow_other_rec: 是否允许消息分发到其他不同类名装饰器(该参数只对同一个类 kind中的不同函数类装饰器有效
                            同步和异步装饰器属于互不干扰类型, 每个大类 kind中的参数只对该大类中的函数类装饰器有效)
    :param judge_msg: 判断是否为该装饰器所处理的消息的函数
    """
    if func in self._message_callback_func_list: return
    self._message_callback_func_list.append(func)
    self._cbFunc[func] = {
        'bot': bot,
        'kind': kind,
        'func_kind': register_name,
        'allow_other_rec': allow_other_rec,
        'judge_msg': judge_msg,
    }


def _run_func(self):
    """
    消息分发器, 将消息发送给所有消息处理函数
    """
    try:
        async_func = []
        universal_func = []
        # 将装饰器中的函数分为异步和同步
        for ele in self._cbFunc:
            if asyncio.iscoroutinefunction(ele):
                async_func.append(ele)
            else:
                universal_func.append(ele)

        # 支持在这里自定义函数过滤消息, 以及使得否分发给所有函数
        def filter_message(universal_func_list, async_func_list, msg):
            """
            根据每个装饰器的过滤要求过滤消息, 以及对分发的函数的限制
            """
            # 对分类后的函数, 分别进行消息函数 函数类分类
            _join_thread_func = []
            _join_loop_func = []
            # 对函数类分类结果判断是否分类过, 若分过类直接跳过, 拒绝再次分类, 否则进行一遍筛选
            if not self._inCache:
                # 对传入的过滤函数按照 kind进行分组
                for f_ele in self._cbFunc:
                    if self._kind_dict[self._cbFunc[f_ele]['kind']].get(self._cbFunc[f_ele]['func_kind'], None) is None:
                        self._kind_dict[self._cbFunc[f_ele]['kind']][self._cbFunc[f_ele]['func_kind']] = {
                            'kind': self._cbFunc[f_ele]['kind'],
                            'func_kind': self._cbFunc[f_ele]['func_kind'],
                            'allow_other_rec': self._cbFunc[f_ele]['allow_other_rec'],
                            'judge_msg': self._cbFunc[f_ele]['judge_msg'],
                        }
                        self._kind_dict[self._cbFunc[f_ele]['kind']][self._cbFunc[f_ele]['func_kind']]['fun'] = []
                        self._kind_dict[self._cbFunc[f_ele]['kind']][self._cbFunc[f_ele]['func_kind']]['fun'].append(f_ele)
                    if f_ele not in (li := self._kind_dict[self._cbFunc[f_ele]['kind']][self._cbFunc[f_ele]['func_kind']]['fun']):
                        li.append(f_ele)

            self._inCache = True

            # 判断任意两个大类 kind 中是否存在全函数类 allow_other_rec 都为 True 的情况, 若出现, 则不进行该大类的函数的消息限制分发, 消息分发给所有函数类
            if len(lis := ([al_ele['allow_other_rec'] for al_ele in self._kind_dict['async'].values()])) == 1 and lis[0]:
                self._loop_flag = True
                _join_loop_func = async_func_list

            if len(lis := ([al_ele['allow_other_rec'] for al_ele in self._kind_dict['universal'].values()])) == 1 and lis[0]:
                self._thread_flag = True
                _join_thread_func = universal_func_list

            # 进行仔细过滤, 对有消息分发限制的函数进行识别, 只把消息分发给符合限制的函数
            for f_ele in self._kind_dict.values():
                for k_ele in f_ele.values():
                    if k_ele['kind'] == 'async' and not self._loop_flag:
                        if k_ele['judge_msg'](msg):
                            if not k_ele['allow_other_rec']:
                                _join_loop_func = self._kind_dict[k_ele['kind']][k_ele['func_kind']]['fun']
                                break
                            else:
                                _join_loop_func.extend(self._kind_dict[k_ele['kind']][k_ele['func_kind']]['fun'])
                    elif k_ele['kind'] == 'universal' and not self._thread_flag:
                        if k_ele['judge_msg'](msg) and not k_ele['allow_other_rec']:
                            if not k_ele['allow_other_rec']:
                                _join_thread_func = self._kind_dict[k_ele['kind']][k_ele['func_kind']]['fun']
                                break
                            else:
                                _join_thread_func.extend(self._kind_dict[k_ele['kind']][k_ele['func_kind']]['fun'])
            return _join_thread_func, _join_loop_func

        universal_func, async_func = filter_message(universal_func, async_func, self._message)

        # 同步函数运行器
        def run_universal_func():
            for fn in universal_func:
                fn(self._cbFunc[fn]['bot'], self._message)

        if len(universal_func) != 0: Thread(target=run_universal_func).start()
        if len(async_func) == 0: return

        # 异步函数运行器
        async def _run_callback():
            _tasks = [asyncio.create_task(
                a_ele(
                    self._cbFunc[a_ele]['bot'], self._message)) for a_ele in async_func]
            return await asyncio.wait(_tasks)

        self._loop.run_until_complete(_run_callback())
    except:
        traceback.print_exc()
