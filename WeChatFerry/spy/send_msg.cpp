#include "framework.h"
#include <sstream>
#include <vector>

#include "exec_sql.h"
#include "log.h"
#include "send_msg.h"
#include "spy_types.h"
#include "util.h"

extern HANDLE g_hEvent;
extern QWORD g_WeChatWinDllAddr;
extern string GetSelfWxid(); // Defined in spy.cpp

#define SRTM_SIZE 0x3F0

#define OS_NEW             0x1C28800
#define OS_FREE            0x1C1FF10
#define OS_SEND_MSG_MGR    0x1C1E690
#define OS_SEND_TEXT       0x238DDD0
#define OS_SEND_IMAGE      0x2383560
#define OS_GET_APP_MSG_MGR 0x1C23630
#define OS_SEND_FILE       0x21969E0
#define OS_RTM_NEW         0x1C27D50
#define OS_RTM_FREE        0x1C27120
#define OS_SEND_RICH_TEXT  0x21A09C0
#define OS_SEND_PAT_MSG    0x2D669B0
#define OS_FORWARD_MSG     0x238D350
#define OS_GET_EMOTION_MGR 0x1C988D0
#define OS_SEND_EMOTION    0x227B9E0

typedef QWORD (*New_t)(QWORD);
typedef QWORD (*Free_t)(QWORD);
typedef QWORD (*SendMsgMgr_t)();
typedef QWORD (*GetAppMsgMgr_t)();
typedef QWORD (*SendTextMsg_t)(QWORD, QWORD, QWORD, QWORD, QWORD, QWORD, QWORD, QWORD);
typedef QWORD (*SendImageMsg_t)(QWORD, QWORD, QWORD, QWORD, QWORD);
typedef QWORD (*SendFileMsg_t)(QWORD, QWORD, QWORD, QWORD, QWORD, QWORD *, QWORD, QWORD *, QWORD, QWORD *, QWORD,
                               QWORD);
typedef QWORD (*SendRichTextMsg_t)(QWORD, QWORD, QWORD);
typedef QWORD (*SendPatMsg_t)(QWORD, QWORD);
typedef QWORD (*ForwardMsg_t)(QWORD, QWORD, QWORD, QWORD);
typedef QWORD (*GetEmotionMgr_t)();
typedef QWORD (*SendEmotion_t)(QWORD, QWORD, QWORD, QWORD, QWORD, QWORD, QWORD, QWORD);

void SendTextMessage(string wxid, string msg, string atWxids)
{
    QWORD success  = 0;
    wstring wsWxid = String2Wstring(wxid);
    wstring wsMsg  = String2Wstring(msg);
    WxString wxMsg(wsMsg);
    WxString wxWxid(wsWxid);

    vector<wstring> vAtWxids;
    vector<WxString> vWxAtWxids;
    if (!atWxids.empty()) {
        wstringstream wss(String2Wstring(atWxids));
        while (wss.good()) {
            wstring wstr;
            getline(wss, wstr, L',');
            vAtWxids.push_back(wstr);
            WxString wxAtWxid(vAtWxids.back());
            vWxAtWxids.push_back(wxAtWxid);
        }
    } else {
        WxString wxEmpty = WxString();
        vWxAtWxids.push_back(wxEmpty);
    }

    QWORD wxAters = (QWORD) & ((RawVector_t *)&vWxAtWxids)->start;

    char buffer[0x460]            = { 0 };
    SendMsgMgr_t funcSendMsgMgr   = (SendMsgMgr_t)(g_WeChatWinDllAddr + OS_SEND_MSG_MGR);
    SendTextMsg_t funcSendTextMsg = (SendTextMsg_t)(g_WeChatWinDllAddr + OS_SEND_TEXT);
    Free_t funcFree               = (Free_t)(g_WeChatWinDllAddr + OS_FREE);
    funcSendMsgMgr();
    success = funcSendTextMsg((QWORD)(&buffer), (QWORD)(&wxWxid), (QWORD)(&wxMsg), wxAters, 1, 1, 0, 0);
    funcFree((QWORD)(&buffer));
}

void SendImageMessage(string wxid, string path)
{
    wstring wsWxid = String2Wstring(wxid);
    wstring wsPath = String2Wstring(path);

    WxString wxWxid(wsWxid);
    WxString wxPath(wsPath);

    New_t funcNew                = (New_t)(g_WeChatWinDllAddr + OS_NEW);
    Free_t funcFree              = (Free_t)(g_WeChatWinDllAddr + OS_FREE);
    SendMsgMgr_t funcSendMsgMgr  = (SendMsgMgr_t)(g_WeChatWinDllAddr + OS_SEND_MSG_MGR);
    SendImageMsg_t funcSendImage = (SendImageMsg_t)(g_WeChatWinDllAddr + OS_SEND_IMAGE);

    char msg[0x460]    = { 0 };
    char msgTmp[0x460] = { 0 };
    QWORD *flag[10]    = { 0 };

    QWORD tmp1 = 0, tmp2 = 0;
    QWORD pMsgTmp = funcNew((QWORD)(&msgTmp));
    flag[8]       = &tmp1;
    flag[9]       = &tmp2;
    flag[1]       = (QWORD *)(pMsgTmp);

    QWORD pMsg    = funcNew((QWORD)(&msg));
    QWORD sendMgr = funcSendMsgMgr();
    funcSendImage(sendMgr, pMsg, (QWORD)(&wxWxid), (QWORD)(&wxPath), (QWORD)(&flag));
    funcFree(pMsg);
    funcFree(pMsgTmp);
}

void SendFileMessage(string wxid, string path)
{
    wstring wsWxid = String2Wstring(wxid);
    wstring wsPath = String2Wstring(path);

    WxString wxWxid(wsWxid);
    WxString wxPath(wsPath);

    New_t funcNew                   = (New_t)(g_WeChatWinDllAddr + OS_NEW);
    Free_t funcFree                 = (Free_t)(g_WeChatWinDllAddr + OS_FREE);
    GetAppMsgMgr_t funcGetAppMsgMgr = (GetAppMsgMgr_t)(g_WeChatWinDllAddr + OS_GET_APP_MSG_MGR);
    SendFileMsg_t funcSendFile      = (SendFileMsg_t)(g_WeChatWinDllAddr + OS_SEND_FILE);

    char msg[0x460] = { 0 };
    QWORD tmp1[4]   = { 0 };
    QWORD tmp2[4]   = { 0 };
    QWORD tmp3[4]   = { 0 };

    QWORD pMsg   = funcNew((QWORD)(&msg));
    QWORD appMgr = funcGetAppMsgMgr();
    funcSendFile(appMgr, pMsg, (QWORD)(&wxWxid), (QWORD)(&wxPath), 1, tmp1, 0, tmp2, 0, tmp3, 0, 0);
    funcFree(pMsg);
}

int SendRichTextMessage(RichText_t &rt)
{ // TODO: Fix memory leak
    QWORD status = -1;

    New_t funcNew                          = (New_t)(g_WeChatWinDllAddr + OS_RTM_NEW);
    Free_t funcFree                        = (Free_t)(g_WeChatWinDllAddr + OS_RTM_FREE);
    GetAppMsgMgr_t funcGetAppMsgMgr        = (GetAppMsgMgr_t)(g_WeChatWinDllAddr + OS_GET_APP_MSG_MGR);
    SendRichTextMsg_t funcForwordPublicMsg = (SendRichTextMsg_t)(g_WeChatWinDllAddr + OS_SEND_RICH_TEXT);

    char *buff = (char *)HeapAlloc(GetProcessHeap(), 0, SRTM_SIZE);
    if (buff == NULL) {
        LOG_ERROR("Out of Memory...");
        return -1;
    }

    memset(buff, 0, SRTM_SIZE);
    funcNew((QWORD)buff);
    WxString *pReceiver = NewWxStringFromStr(rt.receiver);
    WxString *pTitle    = NewWxStringFromStr(rt.title);
    WxString *pUrl      = NewWxStringFromStr(rt.url);
    WxString *pThumburl = NewWxStringFromStr(rt.thumburl);
    WxString *pDigest   = NewWxStringFromStr(rt.digest);
    WxString *pAccount  = NewWxStringFromStr(rt.account);
    WxString *pName     = NewWxStringFromStr(rt.name);

    memcpy(buff + 0x8, pTitle, sizeof(WxString));
    memcpy(buff + 0x48, pUrl, sizeof(WxString));
    memcpy(buff + 0xB0, pThumburl, sizeof(WxString));
    memcpy(buff + 0xF0, pDigest, sizeof(WxString));
    memcpy(buff + 0x2C0, pAccount, sizeof(WxString));
    memcpy(buff + 0x2E0, pName, sizeof(WxString));

    QWORD mgr = funcGetAppMsgMgr();
    status    = funcForwordPublicMsg(mgr, (QWORD)(pReceiver), (QWORD)(buff));
    funcFree((QWORD)buff);

    return (int)status;
}

int SendPatMessage(string roomid, string wxid)
{
    QWORD status = -1;

    wstring wsRoomid = String2Wstring(roomid);
    wstring wsWxid   = String2Wstring(wxid);
    WxString wxRoomid(wsRoomid);
    WxString wxWxid(wsWxid);

    SendPatMsg_t funcSendPatMsg = (SendPatMsg_t)(g_WeChatWinDllAddr + OS_SEND_PAT_MSG);

    status = funcSendPatMsg((QWORD)(&wxRoomid), (QWORD)(&wxWxid));
    return (int)status;
}

int ForwardMessage(QWORD msgid, string receiver)
{
    int status     = -1;
    uint32_t dbIdx = 0;
    QWORD localId  = 0;

    ForwardMsg_t funcForwardMsg = (ForwardMsg_t)(g_WeChatWinDllAddr + OS_FORWARD_MSG);
    if (GetLocalIdandDbidx(msgid, &localId, &dbIdx) != 0) {
        LOG_ERROR("Failed to get localId, Please check id: {}", to_string(msgid));
        return status;
    }

    WxString *pReceiver = NewWxStringFromStr(receiver);

    LARGE_INTEGER l;
    l.HighPart = dbIdx;
    l.LowPart  = (DWORD)localId;

    status = (int)funcForwardMsg((QWORD)pReceiver, l.QuadPart, 0x4, 0x0);

    return status;
}

void SendEmotionMessage(string wxid, string path)
{
    GetEmotionMgr_t GetEmotionMgr = (GetEmotionMgr_t)(g_WeChatWinDllAddr + OS_GET_EMOTION_MGR);
    SendEmotion_t SendEmotion     = (SendEmotion_t)(g_WeChatWinDllAddr + OS_SEND_EMOTION);

    WxString *pWxPath = NewWxStringFromStr(path);
    WxString *pWxWxid = NewWxStringFromStr(wxid);

    QWORD *buff = (QWORD *)HeapAlloc(GetProcessHeap(), 0, 0x20);
    if (buff == NULL) {
        LOG_ERROR("Out of Memory...");
        return;
    }

    memset(buff, 0, 0x20);
    QWORD mgr = GetEmotionMgr();
    SendEmotion(mgr, (QWORD)pWxPath, (QWORD)buff, (QWORD)pWxWxid, 2, (QWORD)buff, 0, (QWORD)buff);
}

#if 0
void SendXmlMessage(string receiver, string xml, string path, int type)
{
    if (g_WeChatWinDllAddr == 0) {
        return;
    }

    // 发送消息Call地址 = 微信基址 + 偏移
    DWORD sendXmlCall1 = g_WeChatWinDllAddr + g_WxCalls.sendXml.call1;
    DWORD sendXmlCall2 = g_WeChatWinDllAddr + g_WxCalls.sendXml.call2;
    DWORD sendXmlCall3 = g_WeChatWinDllAddr + g_WxCalls.sendXml.call3;
    DWORD sendXmlCall4 = g_WeChatWinDllAddr + g_WxCalls.sendXml.call4;
    DWORD sendXmlParam = g_WeChatWinDllAddr + g_WxCalls.sendXml.param;

    char buffer[0xFF0] = { 0 };
    char nullBuf[0x1C] = { 0 };

    wstring wsSender   = String2Wstring(GetSelfWxid());
    wstring wsReceiver = String2Wstring(receiver);
    wstring wsXml      = String2Wstring(xml);

    WxString wxPath;
    WxString wxNull;
    WxString wxXml(wsXml);
    WxString wxSender(wsSender);
    WxString wxReceiver(wsReceiver);

    if (!path.empty()) {
        wstring wsPath = String2Wstring(path);
        wxPath         = WxString(wsPath);
    }

    DWORD sendtype = type;
    __asm {
		pushad;
		pushfd;
		lea ecx, buffer;
		call sendXmlCall1;
		mov eax, [sendtype];
		push eax;
		lea eax, nullBuf;
		lea edx, wxSender;
		push eax;
		lea eax, wxPath;
		push eax;
		lea eax, wxXml;
		push eax;
		lea edi, wxReceiver;
		push edi;
		lea ecx, buffer;
		call sendXmlCall2;
		add esp, 0x14;
		lea eax, wxNull;
		push eax;
		lea ecx, buffer;
		call sendXmlCall3;
		mov dl, 0x0;
		lea ecx, buffer;
		push sendXmlParam;
		push sendXmlParam;
		call sendXmlCall4;
		add esp, 0x8;
		popfd;
		popad;
    }
}
#endif
