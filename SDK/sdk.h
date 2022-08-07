#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

using namespace std;

typedef struct WxMessage {
    int self;        // 是否自己发的消息：0=否，1=是
    int type;        // 消息类型
    int source;      // 消息来源：0=好友消息，1=群消息
    wstring id;      // 消息ID
    wstring xml;     // 群其他消息
    wstring wxId;    // 发送人微信ID
    wstring roomId;  // 群ID
    wstring content; // 消息内容，MAC版最大：16384，即16KB
} WxMessage_t;

typedef struct WxContact {
    wstring wxId;       // 微信ID
    wstring wxCode;     // 微信号
    wstring wxName;     // 微信昵称
    wstring wxCountry;  // 国家
    wstring wxProvince; // 省/州
    wstring wxCity;     // 城市
    wstring wxGender;   // 性别
} WxContact_t;

typedef map<int, wstring> MsgTypesMap_t;
typedef map<wstring, WxContact_t> ContactMap_t;

int WxInitSDK();
int WxSetTextMsgCb(const std::function<int(WxMessage_t)> &onMsg);
int WxSendTextMsg(wstring wxid, wstring at_wxid, wstring msg);
int WxSendImageMsg(wstring wxid, wstring path);
ContactMap_t WxGetContacts();
MsgTypesMap_t WxGetMsgTypes();
vector<wstring> WxGetDbNames();
