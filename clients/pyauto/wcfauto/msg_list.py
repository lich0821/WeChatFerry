# coding: utf-8
# @Author: 小杨大帅哥
import queue
import time
from threading import Thread

class messageList(list):
    def __init__(self, *args, **kwargs):
        super(messageList, self).__init__(*args, **kwargs)
        self.__isRunning = True
        self.__th = None
        self.__time_step = 3*60
        self.__msg_queen = queue.Queue()
        self.start()

    def append(self, item) -> None:
        self.__isRunning = True
        if item['data'].get('msgid', None) is None:
            return
        super(messageList, self).append({str(item['data']['msgid']): item})
        self.__msg_queen.put({'data': item, 'submit_time': time.time()})

    def stop(self):
        self.__isRunning = False

    def find_msg(self, msgid):
        msgid = str(msgid)
        for msg_ele in self:
            if str(msgid) == str(list(msg_ele.keys())[0]):
                return msg_ele[msgid]
        return None

    def start(self):
        def _start():
            while True:
                if self.__isRunning:
                    try:
                        new_data = self.__msg_queen.get()
                        now = time.time()
                        if now - new_data['submit_time'] >= self.__time_step:
                            self.remove({str(new_data['data']['data']['msgid']): new_data['data']})
                            continue
                        time.sleep(self.__time_step - (now - new_data['submit_time']))
                        self.remove({str(new_data['data']['data']['msgid']): new_data['data']})
                    except (queue.Empty, KeyboardInterrupt):
                        pass
        self.__th = Thread(target=_start, name='run', daemon=True)
        self.__th.start()


msg_list = messageList()
