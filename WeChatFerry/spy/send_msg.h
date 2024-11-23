#pragma once

#include <string>

using namespace std;

typedef struct {
    string name;
    string account;
    string title;
    string digest;
    string url;
    string thumburl;
    string receiver;
} RichText_t;

void SendTextMessage(string wxid, string msg, string atWxids);
void SendImageMessage(string wxid, string path);
void SendFileMessage(string wxid, string path);
void SendXmlMessage(string receiver, string xml, string path, uint64_t type);
void SendEmotionMessage(string wxid, string path);
int SendRichTextMessage(RichText_t &rt);
int SendPatMessage(string roomid, string wxid);
int ForwardMessage(uint64_t msgid, string receiver);
