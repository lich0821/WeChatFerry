#pragma execution_character_set("utf-8")

#include "contact_manager.h"

#include "log.hpp"
#include "pb_util.h"
#include "util.h"
#include "offsets.h"
#include "rpc_helper.h"
#include "spy.h"
#include "spy_types.h"

namespace contact
{

namespace OsCnt = Offsets::Contact;

#define FEAT_LEN 5
static const uint8_t FEAT_COUNTRY[FEAT_LEN]  = { 0xA4, 0xD9, 0x02, 0x4A, 0x18 };
static const uint8_t FEAT_PROVINCE[FEAT_LEN] = { 0xE2, 0xEA, 0xA8, 0xD1, 0x18 };
static const uint8_t FEAT_CITY[FEAT_LEN]     = { 0x1D, 0x02, 0x5B, 0xBF, 0x18 };

static uint32_t find_mem(uint32_t start, uint32_t end, const void *target, size_t len)
{
    uint8_t *p = (uint8_t *)start;
    while ((uint32_t)p < end) {
        if (memcmp((void *)p, target, len) == 0) {
            return (uint32_t)p;
        }
        p++;
    }
    return 0;
}

static std::string get_cnt_string(uint32_t start, uint32_t end, const uint8_t *feat, size_t len)
{
    uint32_t pfeat = find_mem(start, end, feat, len);
    if (pfeat == 0) {
        return "";
    }

    uint32_t lfeat = util::get_dword(pfeat + len);
    if (lfeat <= 2) {
        return "";
    }

    return util::w2s(std::wstring(util::get_p_wstring(pfeat + FEAT_LEN + 4), lfeat));
}

std::vector<RpcContact_t> get_contacts()
{
    std::vector<RpcContact_t> contacts;
    uint32_t base = g_WeChatWinDllAddr;
    uint32_t call1 = base + OsCnt::BASE;
    uint32_t call2 = base + OsCnt::HEAD;

    int success    = 0;
    uint32_t *addr[3] = { 0, 0, 0 };
    __asm {
        pushad
        call       call1
        lea        ecx,addr
        push       ecx
        mov        ecx,eax
        call       call2
        mov        success,eax
        popad
    }

    uint32_t pstart = (uint32_t)addr[0];
    uint32_t pend   = (uint32_t)addr[2];

    while (pstart < pend) {
        RpcContact_t cnt;
        uint32_t pbin   = util::get_dword(pstart + 0x150);
        uint32_t lenbin = util::get_dword(pstart + 0x154);

        cnt.wxid   = util::get_string_by_address(pstart + OsCnt::WXID);
        cnt.code   = util::get_string_by_address(pstart + OsCnt::CODE);
        cnt.remark = util::get_string_by_address(pstart + OsCnt::REMARK);
        cnt.name   = util::get_string_by_address(pstart + OsCnt::NAME);

        cnt.country  = get_cnt_string(pbin, pbin + lenbin, FEAT_COUNTRY, FEAT_LEN);
        cnt.province = get_cnt_string(pbin, pbin + lenbin, FEAT_PROVINCE, FEAT_LEN);
        cnt.city     = get_cnt_string(pbin, pbin + lenbin, FEAT_CITY, FEAT_LEN);

        if (pbin == 0) {
            cnt.gender = 0;
        } else {
            cnt.gender = (uint32_t) * (uint8_t *)(pbin + OsCnt::GENDER);
        }

        contacts.push_back(cnt);
        pstart += 0x438;
    }

    return contacts;
}

int accept_new_friend(const std::string &v3, const std::string &v4, int scene)
{
    int success = 0;
    uint32_t base = g_WeChatWinDllAddr;

    uint32_t acceptNewFriendCall1 = base + Offsets::Friend::ACCEPT_CALL1;
    uint32_t acceptNewFriendCall2 = base + Offsets::Friend::ACCEPT_CALL2;
    uint32_t acceptNewFriendCall3 = base + Offsets::Friend::ACCEPT_CALL3;
    uint32_t acceptNewFriendCall4 = base + Offsets::Friend::ACCEPT_CALL4;

    char buffer[0x40]      = { 0 };
    char nullbuffer[0x3CC] = { 0 };

    LOG_DEBUG("v3: {}, v4: {}, scene: {}", v3, v4, scene);

    std::wstring wsV3 = util::s2w(v3);
    std::wstring wsV4 = util::s2w(v4);
    WxString wxV3(wsV3);
    WxString wxV4(wsV4);

    __asm {
        pushad;
        pushfd;
        lea ecx, buffer;
        call acceptNewFriendCall1;
        mov esi, 0x0;
        mov edi, scene;
        push esi;
        push edi;
        sub esp, 0x14;
        mov ecx, esp;
        lea eax, wxV4;
        push eax;
        call acceptNewFriendCall2;
        sub esp, 0x8;
        push 0x0;
        lea eax, nullbuffer;
        push eax;
        lea eax, wxV3;
        push eax;
        lea ecx, buffer;
        call acceptNewFriendCall3;
        mov success, eax;
        lea ecx, buffer;
        call acceptNewFriendCall4;
        popfd;
        popad;
    }

    return success;
}

int add_friend_by_wxid(const std::string &wxid, const std::string &msg)
{
    // TODO: 实现添加好友功能
    return 0;
}

RpcContact_t get_contact_by_wxid(const std::string &wxid)
{
    RpcContact_t contact;
    char buff[0x440] = { 0 };
    std::wstring wsWxid   = util::s2w(wxid);
    WxString pri(wsWxid);
    uint32_t base = g_WeChatWinDllAddr;
    uint32_t contact_mgr_addr  = base + 0x75A4A0;
    uint32_t get_contact_addr  = base + 0xC04E00;
    uint32_t free_contact_addr = base + 0xEA7880;
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
    contact.code   = util::get_string_by_wstr_addr((uint32_t)buff + OsCnt::CODE);
    contact.remark = util::get_string_by_wstr_addr((uint32_t)buff + OsCnt::REMARK);
    contact.name   = util::get_string_by_wstr_addr((uint32_t)buff + OsCnt::NAME);
    contact.gender = util::get_dword((uint32_t)buff + 0x148);

    __asm {
        PUSHAD
        PUSHFD
        LEA        ECX,buff
        CALL       free_contact_addr
        POPFD
        POPAD
    }

    return contact;
}

bool rpc_get_contacts(uint8_t *out, size_t *len)
{
    std::vector<RpcContact_t> contacts = get_contacts();
    return fill_response<Functions_FUNC_GET_CONTACTS>(out, len, contacts, [](Response &rsp, auto &contacts) {
        rsp.msg.contacts.contacts.funcs.encode = encode_contacts;
        rsp.msg.contacts.contacts.arg          = &contacts;
    });
}

bool rpc_get_contact_info(const std::string &wxid, uint8_t *out, size_t *len)
{
    std::vector<RpcContact_t> contacts;
    if (!wxid.empty()) {
        contacts.push_back(get_contact_by_wxid(wxid));
    }
    return fill_response<Functions_FUNC_GET_CONTACT_INFO>(out, len, contacts, [](Response &rsp, auto &contacts) {
        rsp.msg.contacts.contacts.funcs.encode = encode_contacts;
        rsp.msg.contacts.contacts.arg          = &contacts;
    });
}

bool rpc_accept_friend(const std::string &v3, const std::string &v4, int scene, uint8_t *out, size_t *len)
{
    int result = accept_new_friend(v3, v4, scene);
    return fill_response<Functions_FUNC_ACCEPT_FRIEND>(out, len, [result](Response &rsp) {
        rsp.msg.status = result;
    });
}

} // namespace contact
