#pragma once

extern "C" {
__declspec(dllexport) int WxInitSDK(bool debug, int port);
__declspec(dllexport) int WxDestroySDK();
}