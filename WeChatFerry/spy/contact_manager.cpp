#pragma execution_character_set("utf-8")

#include "contact_manager.h"

#include "log.hpp"
#include "offsets.h"
#include "pb_util.h"
#include "rpc_helper.h"
#include "spy.h"
#include "util.h"

using namespace std;

namespace contact
{
namespace OsCon = Offsets::Contact;

using get_contact_mgr_t  = QWORD (*)();
using get_contact_list_t = QWORD (*)(QWORD, QWORD);
using func_verify_new_t  = QWORD (*)(QWORD, WxString *);
using func_verify_ok_t   = QWORD (*)(QWORD, WxString *, QWORD *, QWORD, QWORD, QWORD, QWORD, QWORD, WxString *);

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
    auto func_get_contact_mgr  = Spy::getFunction<get_contact_mgr_t>(OsCon::MGR);
    auto func_get_contact_list = Spy::getFunction<get_contact_list_t>(OsCon::LIST);

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
        QWORD pbin   = util::get_qword(pstart + OsCon::BIN);
        QWORD lenbin = util::get_dword(pstart + OsCon::BIN_LEN);

        cnt.wxid   = util::get_str_by_wstr_addr(pstart + OsCon::WXID);
        cnt.code   = util::get_str_by_wstr_addr(pstart + OsCon::CODE);
        cnt.remark = util::get_str_by_wstr_addr(pstart + OsCon::REMARK);
        cnt.name   = util::get_str_by_wstr_addr(pstart + OsCon::NAME);

        cnt.country  = get_cnt_string(pbin, pbin + lenbin, FEAT_COUNTRY, FEAT_LEN);
        cnt.province = get_cnt_string(pbin, pbin + lenbin, FEAT_PROVINCE, FEAT_LEN);
        cnt.city     = get_cnt_string(pbin, pbin + lenbin, FEAT_CITY, FEAT_LEN);

        cnt.gender = (pbin == 0) ? 0 : static_cast<DWORD>(*(uint8_t *)(pbin + OsCon::GENDER));

        contacts.push_back(cnt);
        pstart += OsCon::STEP;
    }

    return contacts;
}

int accept_new_friend(const std::string &v3, const std::string &v4, int scene)
{
    // TODO: 备注、标签等
    auto func_new    = Spy::getFunction<func_verify_new_t>(OsCon::VERIFY_NEW);
    auto func_verify = Spy::getFunction<func_verify_ok_t>(OsCon::VERIFY_OK);

    QWORD helper = util::get_qword(Spy::WeChatDll.load() + OsCon::ADD_FRIEND_HELPER);
    QWORD fvdf   = util::get_qword(Spy::WeChatDll.load() + OsCon::FVDF);

    auto pV3 = util::CreateWxString(v3);
    auto pV4 = util::CreateWxString(v4);

    QWORD v4Array[4] = { 0 };
    QWORD pV4Buff    = func_new(reinterpret_cast<QWORD>(&v4Array), pV4);

    char buff[0x100] = { 0 };
    memcpy(buff, &helper, sizeof(&helper));
    QWORD a1 = reinterpret_cast<QWORD>(&buff);

    QWORD ret = func_verify(a1, pV3, &fvdf, 0x1D08B4, pV4Buff, 0x1, pV4Buff, scene, 0x0);
    util::FreeWxString(pV3);
    util::FreeWxString(pV4);

    return static_cast<int>(ret); // 成功返回 1
}

RpcContact_t get_contact_by_wxid(const string &wxid)
{
    RpcContact_t contact;
    LOG_ERROR("技术太菜，实现不了。");
    return contact;
}

bool rpc_get_contacts(uint8_t *out, size_t *len)
{
    vector<RpcContact_t> contacts = get_contacts();
    return fill_response<Functions_FUNC_GET_CONTACTS>(out, len, [&](Response &rsp) {
        rsp.msg.contacts.contacts.funcs.encode = encode_contacts;
        rsp.msg.contacts.contacts.arg          = &contacts;
    });
}

bool rpc_get_contact_info(const string &wxid, uint8_t *out, size_t *len)
{
    vector<RpcContact_t> contacts = { get_contact_by_wxid(wxid) };
    return fill_response<Functions_FUNC_GET_CONTACT_INFO>(out, len, [&](Response &rsp) {
        rsp.msg.contacts.contacts.funcs.encode = encode_contacts;
        rsp.msg.contacts.contacts.arg          = &contacts;
    });
}

bool rpc_accept_friend(const Verification &v, uint8_t *out, size_t *len)
{
    const string v3 = v.v3 ? v.v3 : "";
    const string v4 = v.v4 ? v.v4 : "";
    int scene       = v.scene;
    return fill_response<Functions_FUNC_ACCEPT_FRIEND>(
        out, len, [&](Response &rsp) { rsp.msg.status = accept_new_friend(v3, v4, scene); });
}

} // namespace contact
