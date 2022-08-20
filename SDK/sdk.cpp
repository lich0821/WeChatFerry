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

static DWORD WeChatPID            = 0;
static WCHAR SpyDllPath[MAX_PATH] = { 0 };

int WxInitSDK()
{
    int status           = 0;
    unsigned long ulCode = 0;

    GetModuleFileName(GetModuleHandle(WECHATSDKDLL), SpyDllPath, MAX_PATH);
    PathRemoveFileSpec(SpyDllPath);
    PathAppend(SpyDllPath, WECHATINJECTDLL);

    if (!PathFileExists(SpyDllPath)) {
        return ERROR_FILE_NOT_FOUND;
    }

    status = OpenWeChat(&WeChatPID);
    if (status != 0) {
        return status;
    }

    Sleep(2000); // 等待微信打开
    if (InjectDll(WeChatPID, SpyDllPath)) {
        return -1;
    }

    Sleep(1000); // 等待SPY就绪
    status = RpcConnectServer();
    if (status != 0) {
        printf("RpcConnectServer: %d\n", status);
        return -1;
    }

    do {
        status = RpcIsLogin();
        if (status == -1) {
            return status;
        }
        else if (status == 1) {
            break;
        }
        Sleep(1000);
    } while (1);

    return ERROR_SUCCESS;
}

int WxDestroySDK()
{
    WxDisableRecvMsg();
    RpcDisconnectServer();
    // 关闭 RPC，但不卸载 DLL，方便下次使用。
    //EjectDll(WeChatPID, SpyDllPath);

    return ERROR_SUCCESS;
}

int WxEnableRecvMsg(const std::function<int(WxMessage_t)> &onMsg)
{
    if (onMsg) {
        HANDLE msgThread;
        g_cbReceiveTextMsg = onMsg;
        msgThread = (HANDLE)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RpcEnableReceiveMsg, NULL, 0, NULL);
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

int WxDisableRecvMsg()
{
    RpcDisableReceiveMsg();
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

MsgTypesMap_t WxGetMsgTypes()
{
    static MsgTypesMap_t WxMsgTypes;
    if (WxMsgTypes.empty()) {
        int size            = 0;
        PPRpcIntBstrPair pp = RpcGetMsgTypes(&size);
        for (int i = 0; i < size; i++) {
            WxMsgTypes.insert(make_pair(pp[i]->key, GetWstringFromBstr(pp[i]->value)));
            midl_user_free(pp[i]);
        }
        if (pp) {
            midl_user_free(pp);
        }
    }

    return WxMsgTypes;
}

ContactMap_t WxGetContacts()
{
    ContactMap_t mContact;
    int size        = 0;
    PPRpcContact pp = RpcGetContacts(&size);
    for (int i = 0; i < size; i++) {
        WxContact_t contact;
        contact.wxId       = GetWstringFromBstr(pp[i]->wxId);
        contact.wxCode     = GetWstringFromBstr(pp[i]->wxCode);
        contact.wxName     = GetWstringFromBstr(pp[i]->wxName);
        contact.wxCountry  = GetWstringFromBstr(pp[i]->wxCountry);
        contact.wxProvince = GetWstringFromBstr(pp[i]->wxProvince);
        contact.wxCity     = GetWstringFromBstr(pp[i]->wxCity);
        contact.wxGender   = GetWstringFromBstr(pp[i]->wxGender);

        mContact.insert(make_pair(contact.wxId, contact));
        midl_user_free(pp[i]);
    }

    if (pp) {
        midl_user_free(pp);
    }

    return mContact;
}

std::vector<std::wstring> WxGetDbNames()
{
    std::vector<std::wstring> vDbs;
    int size    = 0;
    BSTR *pBstr = RpcGetDbNames(&size);
    for (int i = 0; i < size; i++) {
        vDbs.push_back(GetWstringFromBstr(pBstr[i]));
    }

    if (pBstr) {
        midl_user_free(pBstr);
    }

    return vDbs;
}

DbTableVector_t WxGetDbTables(wstring db)
{
    DbTableVector_t vTables;
    int size       = 0;
    PPRpcTables pp = RpcGetDbTables(db.c_str(), &size);
    for (int i = 0; i < size; i++) {
        WxDbTable_t tbl;
        tbl.table = GetWstringFromBstr(pp[i]->table);
        tbl.sql   = GetWstringFromBstr(pp[i]->sql);

        vTables.push_back(tbl);
        midl_user_free(pp[i]);
    }

    if (pp) {
        midl_user_free(pp);
    }

    return vTables;
}
