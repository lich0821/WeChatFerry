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
    DWORD recvTransferCall = g_WeChatWinDllAddr + g_WxCalls.tf;

    WxString_t wxWxid = { 0 };
    wstring wsWxid    = String2Wstring(wxid);
    wxWxid.text       = (wchar_t *)wsWxid.c_str();
    wxWxid.size       = wsWxid.size();
    wxWxid.capacity   = wsWxid.capacity();

    WxString_t wxTid = { 0 };
    wstring wsTid    = String2Wstring(wxid);
    wxTid.text       = (wchar_t *)wsTid.c_str();
    wxTid.size       = wsTid.size();
    wxTid.capacity   = wsTid.capacity();

    LOG_DEBUG("Receiving transfer, from: {}, transferid: {}", wxid, transferid);
    __asm {

    }

    return rv;
}
