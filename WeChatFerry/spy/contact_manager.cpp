#pragma execution_character_set("utf-8")

#include "contact_manager.h"

#include "log.hpp"
#include "pb_util.h"
#include "rpc_helper.h"
#include "util.h"

using namespace std;

extern QWORD g_WeChatWinDllAddr;

namespace contact_mgmt
{
#define OS_GET_CONTACT_MGR  0x1B417A0
#define OS_GET_CONTACT_LIST 0x219ED10
#define OS_CONTACT_BIN      0x200
#define OS_CONTACT_BIN_LEN  0x208
#define OS_CONTACT_WXID     0x10
#define OS_CONTACT_CODE     0x30
#define OS_CONTACT_REMARK   0x80
#define OS_CONTACT_NAME     0xA0
#define OS_CONTACT_GENDER   0x0E
#define OS_CONTACT_STEP     0x6A8

using get_contact_mgr_t  = QWORD (*)();
using get_contact_list_t = QWORD (*)(QWORD, QWORD);

#define FEAT_LEN 5
static const uint8_t FEAT_COUNTRY[FEAT_LEN]  = { 0xA4, 0xD9, 0x02, 0x4A, 0x18 };
static const uint8_t FEAT_PROVINCE[FEAT_LEN] = { 0xE2, 0xEA, 0xA8, 0xD1, 0x18 };
static const uint8_t FEAT_CITY[FEAT_LEN]     = { 0x1D, 0x02, 0x5B, 0xBF, 0x18 };

static QWORD find_mem(QWORD start, QWORD end, const void *target, size_t len)
{
    uint8_t *p = reinterpret_cast<uint8_t *>(start);
    while (reinterpret_cast<QWORD>(p) < end) {
        if (memcmp(p, target, len) == 0) {
            return reinterpret_cast<QWORD>(p);
        }
        p++;
    }
    return 0;
}

static string get_cnt_string(QWORD start, QWORD end, const uint8_t *feat, size_t len)
{
    QWORD pfeat = find_mem(start, end, feat, len);
    if (pfeat == 0) {
        return "";
    }

    DWORD lfeat = util::get_dword(pfeat + len);
    if (lfeat <= 2) {
        return "";
    }

    return util::w2s(util::get_p_wstring(pfeat + FEAT_LEN + 4, lfeat));
}

vector<RpcContact_t> get_contacts()
{
    vector<RpcContact_t> contacts;
    get_contact_mgr_t func_get_contact_mgr
        = reinterpret_cast<get_contact_mgr_t>(g_WeChatWinDllAddr + OS_GET_CONTACT_MGR);
    get_contact_list_t func_get_contact_list
        = reinterpret_cast<get_contact_list_t>(g_WeChatWinDllAddr + OS_GET_CONTACT_LIST);

    QWORD mgr     = func_get_contact_mgr();
    QWORD addr[3] = { 0 };
    if (func_get_contact_list(mgr, reinterpret_cast<QWORD>(addr)) != 1) {
        LOG_ERROR("get_contacts failed");
        return contacts;
    }

    QWORD pstart = addr[0];
    QWORD pend   = addr[2];
    while (pstart < pend) {
        RpcContact_t cnt;
        QWORD pbin   = util::get_qword(pstart + OS_CONTACT_BIN);
        QWORD lenbin = util::get_dword(pstart + OS_CONTACT_BIN_LEN);

        cnt.wxid   = util::get_str_by_wstr_addr(pstart + OS_CONTACT_WXID);
        cnt.code   = util::get_str_by_wstr_addr(pstart + OS_CONTACT_CODE);
        cnt.remark = util::get_str_by_wstr_addr(pstart + OS_CONTACT_REMARK);
        cnt.name   = util::get_str_by_wstr_addr(pstart + OS_CONTACT_NAME);

        cnt.country  = get_cnt_string(pbin, pbin + lenbin, FEAT_COUNTRY, FEAT_LEN);
        cnt.province = get_cnt_string(pbin, pbin + lenbin, FEAT_PROVINCE, FEAT_LEN);
        cnt.city     = get_cnt_string(pbin, pbin + lenbin, FEAT_CITY, FEAT_LEN);

        cnt.gender = (pbin == 0) ? 0 : static_cast<DWORD>(*(uint8_t *)(pbin + OS_CONTACT_GENDER));

        contacts.push_back(cnt);
        pstart += OS_CONTACT_STEP;
    }

    return contacts;
}

int accept_new_friend(const std::string &v3, const std::string &v4, int scene)
{
    int success = -1;
#if 0
    DWORD accept_new_friend_call1 = g_WeChatWinDllAddr + g_WxCalls.anf.call1;
    DWORD accept_new_friend_call2 = g_WeChatWinDllAddr + g_WxCalls.anf.call2;
    DWORD accept_new_friend_call3 = g_WeChatWinDllAddr + g_WxCalls.anf.call3;
    DWORD accept_new_friend_call4 = g_WeChatWinDllAddr + g_WxCalls.anf.call4;

    char buffer[0x40]      = { 0 };
    char nullbuffer[0x3CC] = { 0 };

    LOG_DEBUG("\nv3: {}\nv4: {}\nscene: {}", v3, v4, scene);

    wstring ws_v3 = util::s2w(v3);
    wstring ws_v4 = util::s2w(v4);
    WxString wx_v3(ws_v3);
    WxString wx_v4(ws_v4);

    __asm {
        pushad;
        pushfd;
        lea ecx, buffer;
        call accept_new_friend_call1;
        mov esi, 0x0;
        mov edi, scene;
        push esi;
        push edi;
        sub esp, 0x14;
        mov ecx, esp;
        lea eax, wx_v4;
        push eax;
        call accept_new_friend_call2;
        sub esp, 0x8;
        push 0x0;
        lea eax, nullbuffer;
        push eax;
        lea eax, wx_v3;
        push eax;
        lea ecx, buffer;
        call accept_new_friend_call3;
        mov success, eax;
        lea ecx, buffer;
        call accept_new_friend_call4;
        popfd;
        popad;
    }
#endif
    return success; // 成功返回 1
}

RpcContact_t get_contact_by_wxid(const string &wxid)
{
    RpcContact_t contact;
#if 0
    char buff[0x440] = { 0 };
    wstring ws_wxid  = util::s2w(wxid);
    WxString pri(ws_wxid);

    DWORD contact_mgr_addr  = g_WeChatWinDllAddr + 0x75A4A0;
    DWORD get_contact_addr  = g_WeChatWinDllAddr + 0xC04E00;
    DWORD free_contact_addr = g_WeChatWinDllAddr + 0xEA7880;

    __asm {
        PUSHAD
        PUSHFD
        CALL       contact_mgr_addr
        LEA        ECX,buff
        PUSH       ECX
        LEA        ECX,pri
        PUSH       ECX
        MOV        ECX,EAX
        CALL       get_contact_addr
        POPFD
        POPAD
    }

    contact.wxid   = wxid;
    contact.code   = util::get_str_by_wstr_addr(reinterpret_cast<DWORD>(buff) + g_WxCalls.contact.wxCode);
    contact.remark = util::get_str_by_wstr_addr(reinterpret_cast<DWORD>(buff) + g_WxCalls.contact.wxRemark);
    contact.name   = util::get_str_by_wstr_addr(reinterpret_cast<DWORD>(buff) + g_WxCalls.contact.wxName);
    contact.gender = util::get_dword(reinterpret_cast<DWORD>(buff) + 0x148);

    __asm {
        PUSHAD
        PUSHFD
        LEA        ECX,buff
        CALL       free_contact_addr
        POPFD
        POPAD
    }
#endif
    return contact;
}

bool rpc_get_contacts(uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_GET_CONTACTS>(out, len, [](Response &rsp) {
        vector<RpcContact_t> contacts          = get_contacts();
        rsp.msg.contacts.contacts.funcs.encode = encode_contacts;
        rsp.msg.contacts.contacts.arg          = &contacts;
    });
}

bool rpc_get_contact_info(const string &wxid, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_GET_CONTACT_INFO>(out, len, [&](Response &rsp) {
        vector<RpcContact_t> contacts          = { get_contact_by_wxid(wxid) };
        rsp.msg.contacts.contacts.funcs.encode = encode_contacts;
        rsp.msg.contacts.contacts.arg          = &contacts;
    });
}

bool rpc_accept_friend(const string &v3, const string &v4, int scene, uint8_t *out, size_t *len)
{
    return fill_response<Functions_FUNC_ACCEPT_FRIEND>(
        out, len, [&](Response &rsp) { rsp.msg.status = accept_new_friend(v3, v4, scene); });
}

} // namespace contact_mgmt
