#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

#include "MinHook.h"

#include "pb_types.h"
#include "spy_types.h"

namespace message
{

class Handler
{
public:
    static Handler &getInstance();

    // 0: 成功, -1: 失败, 1: 已经开启
    int EnableLog();
    int DisableLog();
    int ListenPyq();
    int UnListenPyq();
    int ListenMsg();
    int UnListenMsg();

    MsgTypes_t GetMsgTypes();

    bool isLoggingEnabled() const { return isLogging.load(); }
    bool isMessageListening() const { return isListeningMsg.load(); }
    bool isPyqListening() const { return isListeningPyq.load(); }

    std::optional<WxMsg_t> popMessage();
    std::condition_variable &getConditionVariable() { return cv_; };
    std::mutex &getMutex() { return mutex_; };

    bool rpc_get_msg_types(uint8_t *out, size_t *len);

private:
    Handler();
    ~Handler();

    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<WxMsg_t> msgQueue_;

    std::atomic<bool> isLogging { false };
    std::atomic<bool> isListeningMsg { false };
    std::atomic<bool> isListeningPyq { false };

    using funcRecvMsg_t = QWORD (*)(QWORD, QWORD);
    using funcWxLog_t   = QWORD (*)(QWORD, QWORD, QWORD, QWORD, QWORD, QWORD, QWORD, QWORD, QWORD, QWORD, QWORD, QWORD);
    using funcRecvPyq_t = QWORD (*)(QWORD, QWORD, QWORD);

    uint32_t *pLogLevel;
    funcWxLog_t funcWxLog, realWxLog;
    funcRecvMsg_t funcRecvMsg, realRecvMsg;
    funcRecvPyq_t funcRecvPyq, realRecvPyq;

    bool isMH_Initialized { false };

    MH_STATUS InitializeHook();
    MH_STATUS UninitializeHook();

    static QWORD DispatchMsg(QWORD arg1, QWORD arg2);
    static QWORD PrintWxLog(QWORD a1, QWORD a2, QWORD a3, QWORD a4, QWORD a5, QWORD a6, QWORD a7, QWORD a8, QWORD a9,
                            QWORD a10, QWORD a11, QWORD a12);
    static QWORD DispatchPyq(QWORD arg1, QWORD arg2, QWORD arg3);
};

} // namespace message
