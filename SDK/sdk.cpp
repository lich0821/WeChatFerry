#include "Shlwapi.h"
#include "framework.h"
#include <process.h>
#include <queue>
#include <stdio.h>
#include <stdlib.h>
#include <tlhelp32.h>
#include <vector>

#include "../Rpc/rpc_h.h"
#pragma comment(lib, "Rpcrt4.lib")

#include "injector.h"
#include "rpc_types.h"
#include "sdk.h"
#include "util.h"

static HANDLE hEvent;
static std::queue<RpcMessage_t> MsgQueue;
static RPC_WSTR pszStringBinding = NULL;
static std::function<int(WxMessage_t)> cbReceiveTextMsg;
static const MsgTypesMap_t WxMsgTypes = MsgTypesMap_t { { 0x01, L"文字" },
                                                        { 0x03, L"图片" },
                                                        { 0x22, L"语音" },
                                                        { 0x25, L"好友确认" },
                                                        { 0x28, L"POSSIBLEFRIEND_MSG" },
                                                        { 0x2A, L"名片" },
                                                        { 0x2B, L"视频" },
                                                        { 0x2F, L"石头剪刀布 | 表情图片" },
                                                        { 0x30, L"位置" },
                                                        { 0x31, L"共享实时位置、文件、转账、链接" },
                                                        { 0x32, L"VOIPMSG" },
                                                        { 0x33, L"微信初始化" },
                                                        { 0x34, L"VOIPNOTIFY" },
                                                        { 0x35, L"VOIPINVITE" },
                                                        { 0x3E, L"小视频" },
                                                        { 0x270F, L"SYSNOTICE" },
                                                        { 0x2710, L"红包、系统消息" },
                                                        { 0x2712, L"撤回消息" } };

RPC_STATUS RpcConnectServer()
{
    RPC_STATUS status = 0;
    // Creates a string binding handle.
    status = RpcStringBindingCompose(NULL,                                             // UUID to bind to
                                     reinterpret_cast<RPC_WSTR>((RPC_WSTR)L"ncalrpc"), // Use TCP/IP protocol
                                     NULL,                                             // TCP/IP network address to use
                                     reinterpret_cast<RPC_WSTR>((RPC_WSTR)L"tmp_endpoint"), // TCP/IP port to use
                                     NULL,               // Protocol dependent network options to use
                                     &pszStringBinding); // String binding output

    if (status)
        return status;

    /* Validates the format of the string binding handle and converts it to a binding handle.
    pszStringBinding: The string binding to validate
    hSpyBinding: Put the result in the implicit binding(defined in the IDL file)
    */
    status = RpcBindingFromStringBinding(pszStringBinding, &hSpyBinding);

    return status;
}

RPC_STATUS RpcDisconnectServer()
{
    RPC_STATUS status;
    // Free the memory allocated by a string
    status = RpcStringFree(&pszStringBinding);
    if (status)
        return status;

    // Releases binding handle resources and disconnects from the server
    status = RpcBindingFree(&hSpyBinding);

    return status;
}

int WxInitSDK()
{
    int loginFlag           = 0;
    unsigned long ulCode    = 0;
    DWORD status            = 0;
    DWORD pid               = 0;
    WCHAR DllPath[MAX_PATH] = { 0 };

    GetModuleFileNameW(GetModuleHandleW(WECHATSDKDLL), DllPath, MAX_PATH);
    PathRemoveFileSpecW(DllPath);
    PathAppendW(DllPath, WECHATINJECTDLL);

    if (!PathFileExistsW(DllPath)) {
        return ERROR_FILE_NOT_FOUND;
    }

    status = OpenWeChat(&pid);
    if (status != 0) {
        return status;
    }
    Sleep(2000); // 等待微信打开
    if (!InjectDll(pid, DllPath)) {
        return -1;
    }

    RpcConnectServer();

    while (!loginFlag) {
        RpcTryExcept
        {
            // 查询登录状态
            loginFlag = client_IsLogin();
        }
        RpcExcept(1)
        {
            ulCode = RpcExceptionCode();
            printf("Runtime reported exception 0x%lx = %ld\n", ulCode, ulCode);
        }
        RpcEndExcept

            Sleep(1000);
    }

    return ERROR_SUCCESS;
}

static unsigned int __stdcall waitForMsg(void *p)
{
    RpcMessage_t *rpcMsg;
    while (true) {
        // 中断式，兼顾及时性和CPU使用率
        WaitForSingleObject(hEvent, INFINITE); // 等待消息
        while (!MsgQueue.empty()) {
            rpcMsg = (RpcMessage_t *)&MsgQueue.front();
            WxMessage_t msg;
            msg.id      = wstring(rpcMsg->id);
            msg.self    = rpcMsg->self;
            msg.type    = rpcMsg->type;
            msg.source  = rpcMsg->source;
            msg.xml     = wstring(rpcMsg->xml);
            msg.wxId    = wstring(rpcMsg->wxId);
            msg.roomId  = wstring(rpcMsg->roomId);
            msg.content = wstring(rpcMsg->content);

            try {
                cbReceiveTextMsg(msg); // 调用接收消息回调
            } catch (...) {
                printf("callback error...\n");
            }
            MsgQueue.pop();
        }
        ResetEvent(hEvent);
    }

    return 0;
}

static unsigned int __stdcall innerWxSetTextMsgCb(void *p)
{
    unsigned long ulCode = 0;
    RpcTryExcept
    {
        // 建立RPC通道，让服务端能够调用客户端的回调函数。（该接口会被服务端阻塞直到异常退出）
        client_EnableReceiveMsg();
    }
    RpcExcept(1)
    {
        ulCode = RpcExceptionCode();
        printf("Runtime reported exception 0x%lx = %ld\n", ulCode, ulCode);
    }
    RpcEndExcept

        return 0;
}

int WxSetTextMsgCb(const std::function<int(WxMessage_t)> &onMsg)
{
    if (onMsg) {
        HANDLE msgThread;
        cbReceiveTextMsg = onMsg;
        hEvent           = CreateEvent(NULL, TRUE, FALSE, NULL);
        msgThread        = (HANDLE)_beginthreadex(NULL, 0, waitForMsg, NULL, 0, NULL);
        if (msgThread == NULL) {
            printf("Failed to create message listening thread.\n");
            return -2;
        }
        CloseHandle(msgThread);

        msgThread = (HANDLE)_beginthreadex(NULL, 0, innerWxSetTextMsgCb, NULL, 0, NULL);
        if (msgThread == NULL) {
            printf("Failed to create innerWxRecvTextMsg.\n");
            return -2;
        }
        CloseHandle(msgThread);

        return 0;
    }

    printf("Empty Callback.\n");
    return -1;
}

int server_ReceiveMsg(RpcMessage_t *rpcMsg)
{
    MsgQueue.push(*rpcMsg); // 发送消息
    SetEvent(hEvent);       // 发送消息通知
    return 0;
}

static int innerWxSendTextMsg(const wchar_t *wxid, const wchar_t *at_wxid, const wchar_t *msg)
{
    int ret              = 0;
    unsigned long ulCode = 0;

    RpcTryExcept { ret = client_SendTextMsg(wxid, at_wxid, msg); }
    RpcExcept(1)
    {
        ulCode = RpcExceptionCode();
        printf("Runtime reported exception 0x%lx = %ld\n", ulCode, ulCode);
    }
    RpcEndExcept

        return ret;
}

int WxSendTextMsg(wstring wxid, wstring at_wxid, wstring msg)
{
    return innerWxSendTextMsg(wxid.c_str(), at_wxid.c_str(), msg.c_str());
}

static int innerWxSendImageMsg(const wchar_t *wxid, const wchar_t *path)
{
    int ret              = 0;
    unsigned long ulCode = 0;

    RpcTryExcept { ret = client_SendImageMsg(wxid, path); }
    RpcExcept(1)
    {
        ulCode = RpcExceptionCode();
        printf("Runtime reported exception 0x%lx = %ld\n", ulCode, ulCode);
    }
    RpcEndExcept

        return ret;
}

int WxSendImageMsg(wstring wxid, wstring path) { return innerWxSendImageMsg(wxid.c_str(), path.c_str()); }

static int getAddrHandle(DWORD *addr, HANDLE *handle)
{
    DWORD processID     = 0;
    wstring processName = L"WeChat.exe";
    wstring moduleName  = L"WeChatWin.dll";

    HANDLE hSnapshot    = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe32 = { sizeof(PROCESSENTRY32) };
    while (Process32Next(hSnapshot, &pe32)) {
        wstring strProcess = pe32.szExeFile;
        if (strProcess == processName) {
            processID = pe32.th32ProcessID;
            break;
        }
    }
    CloseHandle(hSnapshot);
    if (processID == 0) {
        printf("Failed to get Process ID\r\n");
        return -1;
    }

    HANDLE hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processID);
    if (hProcessSnapshot == INVALID_HANDLE_VALUE) {
        printf("Failed to get Process Snapshot\r\n");
        return -2;
    }

    MODULEENTRY32 me32;
    SecureZeroMemory(&me32, sizeof(MODULEENTRY32));
    me32.dwSize = sizeof(MODULEENTRY32);
    while (Module32Next(hProcessSnapshot, &me32)) {
        me32.dwSize = sizeof(MODULEENTRY32);

        if (!wcscmp(me32.szModule, moduleName.c_str())) {
            *addr = (DWORD)me32.modBaseAddr;
            break;
        }
    }

    CloseHandle(hProcessSnapshot);
    if (*addr == 0) {
        printf("Failed to get Module Address\r\n");
        return -3;
    }

    *handle = OpenProcess(PROCESS_VM_READ, FALSE, processID);
    if (*handle == 0) {
        printf("Failed to open Process\r\n");
        return -4;
    }

    return 0;
}

ContactMap_t WxGetContacts()
{
    ContactMap_t mContact;
    DWORD moduleBaseAddress;
    HANDLE hProcess;

    if (getAddrHandle(&moduleBaseAddress, &hProcess) != 0) {
        return mContact;
    }

    DWORD Address1 = moduleBaseAddress + 0x1DB9728;
    DWORD Address2 = GetMemoryIntByAddress(hProcess, Address1);
    DWORD Address3 = GetMemoryIntByAddress(hProcess, Address2 + 0x28 + 0xBC);

    vector<DWORD> nodeAddressList;
    nodeAddressList.push_back(Address3);

    DWORD nodeAddress1 = GetMemoryIntByAddress(hProcess, Address3 + 0x0);
    DWORD nodeAddress2 = GetMemoryIntByAddress(hProcess, Address3 + 0x4);
    DWORD nodeAddress3 = GetMemoryIntByAddress(hProcess, Address3 + 0x8);
    if (find(nodeAddressList.begin(), nodeAddressList.end(), nodeAddress1) == nodeAddressList.end())
        nodeAddressList.push_back(nodeAddress1);
    if (find(nodeAddressList.begin(), nodeAddressList.end(), nodeAddress2) == nodeAddressList.end())
        nodeAddressList.push_back(nodeAddress2);
    if (find(nodeAddressList.begin(), nodeAddressList.end(), nodeAddress3) == nodeAddressList.end())
        nodeAddressList.push_back(nodeAddress3);

    unsigned int index = 1;
    while (index < nodeAddressList.size()) {
        WxContact_t contactItem;
        DWORD nodeAddress     = nodeAddressList[index++];
        DWORD checkNullResult = GetMemoryIntByAddress(hProcess, nodeAddress + 0xD);
        if (checkNullResult == 0) {
            index++;
            continue;
        }
        contactItem.wxId       = GetUnicodeInfoByAddress(hProcess, nodeAddress + 0x38);
        contactItem.wxCode     = GetUnicodeInfoByAddress(hProcess, nodeAddress + 0x4C);
        contactItem.wxName     = GetUnicodeInfoByAddress(hProcess, nodeAddress + 0x94);
        contactItem.wxCountry  = GetUnicodeInfoByAddress(hProcess, nodeAddress + 0x1D8);
        contactItem.wxProvince = GetUnicodeInfoByAddress(hProcess, nodeAddress + 0x1EC);
        contactItem.wxCity     = GetUnicodeInfoByAddress(hProcess, nodeAddress + 0x200);
        DWORD gender           = GetMemoryIntByAddress(hProcess, nodeAddress + 0x18C);

        if (gender == 1)
            contactItem.wxGender = L"男";
        else if (gender == 2)
            contactItem.wxGender = L"女";
        else
            contactItem.wxGender = L"未知";

        mContact.insert(make_pair(contactItem.wxId, contactItem));

        DWORD nodeAddress1 = GetMemoryIntByAddress(hProcess, nodeAddress + 0x0);
        DWORD nodeAddress2 = GetMemoryIntByAddress(hProcess, nodeAddress + 0x4);
        DWORD nodeAddress3 = GetMemoryIntByAddress(hProcess, nodeAddress + 0x8);
        if (find(nodeAddressList.begin(), nodeAddressList.end(), nodeAddress1) == nodeAddressList.end())
            nodeAddressList.push_back(nodeAddress1);
        if (find(nodeAddressList.begin(), nodeAddressList.end(), nodeAddress2) == nodeAddressList.end())
            nodeAddressList.push_back(nodeAddress2);
        if (find(nodeAddressList.begin(), nodeAddressList.end(), nodeAddress3) == nodeAddressList.end())
            nodeAddressList.push_back(nodeAddress3);
    }

    CloseHandle(hProcess);

    return mContact;
}

MsgTypesMap_t WxGetMsgTypes() { return WxMsgTypes; }
