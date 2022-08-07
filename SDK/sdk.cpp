#include "Shlwapi.h"
#include "framework.h"
#include <process.h>
#include <queue>
#include <stdio.h>
#include <stdlib.h>
#include <tlhelp32.h>
#include <vector>

#include "injector.h"
#include "rpc_client.h"
#include "sdk.h"
#include "util.h"

std::function<int(WxMessage_t)> g_cbReceiveTextMsg;

int WxInitSDK()
{
    unsigned long ulCode    = 0;
    DWORD status            = 0;
    DWORD pid               = 0;
    WCHAR DllPath[MAX_PATH] = { 0 };

    GetModuleFileName(GetModuleHandle(WECHATSDKDLL), DllPath, MAX_PATH);
    PathRemoveFileSpec(DllPath);
    PathAppend(DllPath, WECHATINJECTDLL);

    if (!PathFileExists(DllPath)) {
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

    while (!RpcIsLogin()) {
        Sleep(1000);
    }

    return ERROR_SUCCESS;
}

int WxSetTextMsgCb(const std::function<int(WxMessage_t)> &onMsg)
{
    if (onMsg) {
        HANDLE msgThread;
        g_cbReceiveTextMsg = onMsg;

        msgThread = (HANDLE)_beginthreadex(NULL, 0, RpcSetTextMsgCb, NULL, 0, NULL);
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

int WxSendTextMsg(wstring wxid, wstring at_wxid, wstring msg)
{
    return RpcSendTextMsg(wxid.c_str(), at_wxid.c_str(), msg.c_str());
}

int WxSendImageMsg(wstring wxid, wstring path) { return RpcSendImageMsg(wxid.c_str(), path.c_str()); }

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
    printf("WxGetContacts\n");
    DWORD baseAddr = moduleBaseAddress + 0x23638F4;
    DWORD tempAddr = GetMemoryIntByAddress(hProcess, baseAddr);
    DWORD head     = GetMemoryIntByAddress(hProcess, tempAddr + 0x4C);
    DWORD node     = GetMemoryIntByAddress(hProcess, head);

    while (node != head) {
        WxContact_t contactItem;
        contactItem.wxId       = GetUnicodeInfoByAddress(hProcess, node + 0x30);
        contactItem.wxCode     = GetUnicodeInfoByAddress(hProcess, node + 0x44);
        contactItem.wxName     = GetUnicodeInfoByAddress(hProcess, node + 0x8C);
        contactItem.wxCountry  = GetUnicodeInfoByAddress(hProcess, node + 0x1D0);
        contactItem.wxProvince = GetUnicodeInfoByAddress(hProcess, node + 0x1E4);
        contactItem.wxCity     = GetUnicodeInfoByAddress(hProcess, node + 0x1F8);
        DWORD gender           = GetMemoryIntByAddress(hProcess, node + 0x184);

        if (gender == 1)
            contactItem.wxGender = L"男";
        else if (gender == 2)
            contactItem.wxGender = L"女";
        else
            contactItem.wxGender = L"未知";

        mContact.insert(make_pair(contactItem.wxId, contactItem));
        node = GetMemoryIntByAddress(hProcess, node);
    }

    CloseHandle(hProcess);

    return mContact;
}

MsgTypesMap_t WxGetMsgTypes()
{
    static MsgTypesMap_t WxMsgTypes;
    if (WxMsgTypes.empty()) {
        int size = 0;
        PPRpcIntBstrPair_t pp = RpcGetMsgTypes(&size);
        for (int i = 0; i < size; i++) {
            WxMsgTypes.insert(make_pair(pp[i]->key, GetWstringFromBstr(pp[i]->value)));
            midl_user_free(pp[i]);
        }
        midl_user_free(pp);
    }

    return WxMsgTypes;
}

