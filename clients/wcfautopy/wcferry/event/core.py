# -*- coding: utf-8 -*-

import traceback
import asyncio
from threading import Thread


def load_function(cls):
    cls._add_callback = _add_callback
    cls._run_func = _run_func
    return cls


def _add_callback(self, func, bot):
    """
    消息处理函数加载器
    :param func: 消息处理函数
    :param args: 消息处理函数参数
    :param kwargs: 消息处理函数参数
    """
    if func in self._message_callback_func_list: return
    self._message_callback_func_list.append(func)
    self._message_callback_func[func] = bot



def _run_func(self):
    """
    消息分发器, 将消息发送给所有消息处理函数
    """
    try:
        async_func = []
        universal_func = []
        for ele in self._message_callback_func:
            if asyncio.iscoroutinefunction(ele):
                async_func.append(ele)
            else:
                universal_func.append(ele)

        # 同步函数运行器
        def run_universal_func():
            for fn in universal_func:
                fn(self._message_callback_func[fn], self._message)

        if len(universal_func) != 0: Thread(target=run_universal_func).start()
        if len(async_func) == 0: return

        # 异步函数运行器
        async def _run_callback():
            tasks = [asyncio.create_task(func(self._message_callback_func[func], self._message))
                     for func in async_func]
            await asyncio.wait(tasks)

        self._loop.run_until_complete(_run_callback())
    except:
        traceback.print_exc()



