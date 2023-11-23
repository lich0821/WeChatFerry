#include "receive_transfer.h"
#include "load_calls.h"
#include "log.h"
#include "util.h"

using namespace std;

extern WxCalls_t g_WxCalls;
extern DWORD g_WeChatWinDllAddr;

int ReceiveTransfer(string wxid, string transferid, string transactionid)
{
    int rv                  = 0;
    DWORD recvTransferCall1 = g_WeChatWinDllAddr + g_WxCalls.tf.call1;
    DWORD recvTransferCall2 = g_WeChatWinDllAddr + g_WxCalls.tf.call2;
    DWORD recvTransferCall3 = g_WeChatWinDllAddr + g_WxCalls.tf.call3;

    char payInfo[0x134] = { 0 };
    wstring wsWxid      = String2Wstring(wxid);
    wstring wsTfid      = String2Wstring(transferid);
    wstring wsTaid      = String2Wstring(transactionid);

    WxString wxWxid(wsWxid);
    WxString wxTfid(wsTfid);
    WxString wxTaid(wsTaid);

    LOG_DEBUG("Receiving transfer, from: {}, transferid: {}, transactionid: {}", wxid, transferid, transactionid);
    __asm {
        pushad;
        lea ecx, payInfo;
        call recvTransferCall1;
        mov dword ptr[payInfo + 0x4], 0x1;
        mov dword ptr[payInfo + 0x4C], 0x1;
        popad;
    }
    memcpy(&payInfo[0x1C], &wxTaid, sizeof(wxTaid));
    memcpy(&payInfo[0x38], &wxTfid, sizeof(wxTfid));

    __asm {
        pushad;
        push 0x1;
        sub esp, 0x8;
        lea edx, wxWxid;
        lea ecx, payInfo;
        call recvTransferCall2;
        mov rv, eax;
        add esp, 0xC;
        push 0x0;
        lea ecx, payInfo;
        call recvTransferCall3;
        popad;
    }

    return rv;
}
