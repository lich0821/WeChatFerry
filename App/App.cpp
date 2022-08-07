#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <process.h>
#include <windows.h>

#include "sdk.h"
/*
#pragma comment(lib, "SDK.lib")
等效为在属性，链接，输入中添加该依赖
*/

void printContacts(ContactMap_t contacts)
{
    wprintf(L"contacts number: %ld\n", contacts.size());
    for (auto it = contacts.begin(); it != contacts.end(); ++it) {
        wprintf(L"%s\t%s\t%s\t%s\t%s\t%s\t%s\r\n", it->second.wxId.c_str(), it->second.wxCode.c_str(),
                it->second.wxName.c_str(), it->second.wxGender.c_str(), it->second.wxCountry.c_str(),
                it->second.wxProvince.c_str(), it->second.wxCity.c_str());
    }
}

int onTextMsg(WxMessage_t msg)
{
    wprintf(L"%s msgType: %d, msgSource: %d, isSelf: %d\n", msg.id.c_str(), msg.type, msg.source, msg.self);
    wprintf(L"%s[%s] >> %s\n", msg.wxId.c_str(), msg.roomId.c_str(), msg.content.c_str());
    wprintf(L"msgSourceXml: %s\n", msg.xml.c_str());

    return 0;
}

int main()
{
    DWORD status     = 0;
    wstring wxid     = L"filehelper"; // 微信ID
    wstring at_wxid  = L"";
    wstring content  = L"这里填写消息内容";
    wstring img_path = L"test.jpg";

    _wsetlocale(LC_ALL, L"chs"); // 这是个大坑，不设置中文直接不见了。。。

    // 获取消息类型
    const MsgTypesMap_t WxMsgTypes = WxGetMsgTypes();
    for (auto it = WxMsgTypes.begin(); it != WxMsgTypes.end(); ++it) {
        wprintf(L"%d: %s\n", it->first, it->second.c_str());
    }

    wprintf(L"WxInitSDK: ");
    status = WxInitSDK();
    wcout << status << endl;
    wprintf(L"%d\n", status);
    if (status != 0) {
        return 0;
    }

    wprintf(L"Message: 接收通知中......\n");
    WxSetTextMsgCb(onTextMsg);

    // 测试发送消息
    wprintf(L"测试发送消息\n");
    WxSendTextMsg(wxid, at_wxid, content);

    // 测试发送照片
    wprintf(L"测试发送照片\n");
    WxSendImageMsg(wxid, img_path);

    Sleep(10000); // 等待10秒

    // 测试获取联系人
    auto mContact = WxGetContacts();
    printContacts(mContact);

    while (1) {
        Sleep(10000); // 休眠，释放CPU
    }
}
