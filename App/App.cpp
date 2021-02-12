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

int onTextMsg(WxMessage_t msg)
{
    try {
        wcout << msg.id << L" msgType: " << msg.type << L", msgSource: " << msg.source << L", isSelf: " << msg.self
              << endl;
        wcout << msg.wxId << L"[" << msg.roomId << L"]" << L" >> " << msg.content << endl;
        wcout << L"msgSourceXml: " << msg.xml << endl;
    } catch (...) {
        wcout << "something wrong..." << endl;
    }
    wcout.flush();
    return 0;
}

int main()
{
    DWORD status    = 0;
    wstring wxid    = L"filehelper"; // 微信ID
    wstring at_wxid = L"";
    wstring content = L"这里填写消息内容";

    //_setmode(_fileno(stdout), _O_WTEXT); // 没有这个wcout遇到一些字符会导致console卡死，用了会导致脱离控制台
    _wsetlocale(LC_ALL, L"chs"); // 这是个大坑，不设置中文直接不见了。。。

    // 获取消息类型
    const MsgTypesMap_t WxMsgTypes = WxGetMsgTypes();
    for (auto it = WxMsgTypes.begin(); it != WxMsgTypes.end(); ++it) {
        wcout << it->first << L": " << it->second << endl;
    }

    status = WxInitSDK();
    wcout << L"WxInitSDK: " << status << endl;
    if (status != 0) {
        return 0;
    }

    wcout << L"Message: 接收通知中......" << endl;
    WxSetTextMsgCb(onTextMsg);

    // 测试消息发送
    WxSendTextMsg(wxid, at_wxid, content);

    while (1) {
        Sleep(10000); // 休眠，释放CPU
    }
}
