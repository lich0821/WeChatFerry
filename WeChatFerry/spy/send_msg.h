#pragma once

#include <string>

using namespace std;

void SendTextMessage(string wxid, string msg, string atWxids);
void SendImageMessage(string wxid, string path);
void SendFileMessage(string wxid, string path);
void SendXmlMessage(string receiver, string xml, string path, int type);
void SendEmotionMessage(string wxid, string path);
