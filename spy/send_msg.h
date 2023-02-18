#pragma once

#include "framework.h"

using namespace std;

void SendTextMessage(wstring wxid, wstring msg, wstring atWxids);
void SendImageMessage(wstring wxid, wstring path);
