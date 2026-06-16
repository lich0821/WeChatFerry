#pragma once

int WxInitSDK(bool debug, int port);
int WxDestroySDK();
void WxSetGuiMode(bool enable); // 开启后错误改用模态弹框，默认写 stderr
