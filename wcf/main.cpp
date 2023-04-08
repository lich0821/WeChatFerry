#include <stdio.h>
#include <string.h>

#include "framework.h"

#include "log.h"
#include "sdk.h"

void help() { LOG_INFO("Usage: \n启动: wcf.exe start url [debug]\n关闭: wcf.exe stop"); }

int main(int argc, char *argv[])
{
    int ret    = -1;
    bool debug = false;

    if ((argc < 2) || (argc > 4)) {
        help();
    } else if (argc == 4) {
        debug = (strcmp(argv[2], "debug") == 0);
    }

    if (strcmp(argv[1], "start") == 0) {
        ret = WxInitSDK(debug, argv[2]);
    } else if (strcmp(argv[1], "stop") == 0) {
        ret = WxDestroySDK();
    } else {
        help();
    }

    return ret;
}