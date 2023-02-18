#pragma once

#include <string>

using namespace std;

void SendTextMessage(string wxid, string msg, string atWxids);
void SendImageMessage(string wxid, string path);
