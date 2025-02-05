#pragma once

#include <memory>
#include <string>
#include <vector>

#include "spy_types.h"
#include "wcf.pb.h"

namespace message
{

class Sender
{
public:
    static Sender &get_instance();

    void send_text(const std::string &wxid, const std::string &msg, const std::string &at_wxids = "");
    void send_image(const std::string &wxid, const std::string &path);
    void send_file(const std::string &wxid, const std::string &path);
    void send_xml(const std::string &receiver, const std::string &xml, const std::string &path, uint64_t type);
    void send_emotion(const std::string &wxid, const std::string &path);
    int send_rich_text(const RichText &rt);
    int send_pat(const std::string &roomid, const std::string &wxid);
    int forward(uint64_t msgid, const std::string &receiver);

    // RPC 方法
    bool rpc_send_text(const TextMsg &text, uint8_t *out, size_t *len);
    bool rpc_send_image(const std::string &path, const std::string &receiver, uint8_t *out, size_t *len);
    bool rpc_send_file(const std::string &path, const std::string &receiver, uint8_t *out, size_t *len);
    bool rpc_send_emotion(const std::string &path, const std::string &receiver, uint8_t *out, size_t *len);
    bool rpc_send_xml(const XmlMsg &rt, uint8_t *out, size_t *len);
    bool rpc_send_rich_text(const RichText &rt, uint8_t *out, size_t *len);
    bool rpc_send_pat(const std::string &roomid, const std::string &wxid, uint8_t *out, size_t *len);
    bool rpc_forward(uint64_t msgid, const std::string &receiver, uint8_t *out, size_t *len);

private:
    Sender();
    ~Sender() = default;

    Sender(const Sender &)            = delete;
    Sender &operator=(const Sender &) = delete;

    using New_t        = QWORD (*)(QWORD);
    using Free_t       = QWORD (*)(QWORD);
    using SendMsgMgr_t = QWORD (*)();
    using GetAppMgr_t  = QWORD (*)();
    using SendText_t   = QWORD (*)(QWORD, QWORD, QWORD, QWORD, QWORD, QWORD, QWORD, QWORD);
    using SendImage_t  = QWORD (*)(QWORD, QWORD, QWORD, QWORD, QWORD);
    using SendFile_t
        = QWORD (*)(QWORD, QWORD, QWORD, QWORD, QWORD, QWORD *, QWORD, QWORD *, QWORD, QWORD *, QWORD, QWORD);
    using SendRichText_t  = QWORD (*)(QWORD, QWORD, QWORD);
    using SendPat_t       = QWORD (*)(QWORD, QWORD);
    using Forward_t       = QWORD (*)(QWORD, QWORD, QWORD, QWORD);
    using GetEmotionMgr_t = QWORD (*)();
    using SendEmotion_t   = QWORD (*)(QWORD, QWORD, QWORD, QWORD, QWORD, QWORD, QWORD, QWORD);
    using XmlBufSign_t    = QWORD (*)(QWORD, QWORD, QWORD);
    using SendXml_t       = QWORD (*)(QWORD, QWORD, QWORD, QWORD, QWORD, QWORD, QWORD, QWORD, QWORD, QWORD);

    New_t func_new;
    Free_t func_free;
    SendMsgMgr_t func_send_msg_mgr;
    GetAppMgr_t func_get_app_mgr;
    SendText_t func_send_text;
    SendImage_t func_send_image;
    SendFile_t func_send_file;
    SendRichText_t func_send_rich_text;
    SendPat_t func_send_pat;
    Forward_t func_forward;
    GetEmotionMgr_t func_get_emotion_mgr;
    SendEmotion_t func_send_emotion;
    XmlBufSign_t func_xml_buf_sign;
    SendXml_t func_send_xml;

    std::unique_ptr<WxString> new_wx_string(const char *str);
    std::unique_ptr<WxString> new_wx_string(const std::string &str);
    std::vector<WxString> parse_wxids(const std::string &wxids);
};

}