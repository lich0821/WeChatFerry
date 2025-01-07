#pragma once

int WxInitSDK(bool debug, int port, int index=-1);
int WxDestroySDK(int index=0);
int EnumWeChatProcess();
void clearAllSDK();
