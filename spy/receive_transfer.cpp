#include "receive_transfer.h"
#include "load_calls.h"
#include "log.h"
#include "util.h"

using namespace std;

extern WxCalls_t g_WxCalls;
extern DWORD g_WeChatWinDllAddr;

int ReceiveTransfer(string wxid, string transferid)
{
    int rv                  = 0;
    DWORD recvTransferCall  = g_WeChatWinDllAddr + g_WxCalls.tf.call1;
    DWORD recvTransferCall2 = g_WeChatWinDllAddr + g_WxCalls.tf.call2;

    wstring wsWxid = String2Wstring(wxid);
    wstring wsTid  = String2Wstring(transferid);

    LOG_DEBUG("Receiving transfer, from: {}, transferid: {}", wxid, transferid);
    __asm {
        pushad
        sub esp, 0x30
        mov ecx, esp
        lea eax, wsTid
        push eax
        call recvTransferCall
        lea ecx, dword ptr ds:[esp+0x14]
        lea eax, wsWxid
        push eax
        call recvTransferCall
        call recvTransferCall2
        add esp, 0x30
        mov rv, eax
        popad
    }

    return rv;
}
