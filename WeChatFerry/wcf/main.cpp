#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sdk.h"

void help()
{
    printf("\nUsage: \n启动: wcf.exe start port [debug]\n关闭: wcf.exe stop\nport: 命令端口, 消息端口为命令端口+1\n指定微信pid启动: wcf.exe start port [pid]仅在你知道这是什么时候使用\n打开新的微信: wcf.exe start port [multi]\n");
}

int main(int argc, char *argv[])
{
    int ret    = -1;
    bool debug = false;
    bool multi = false;

    if ((argc < 2) || (argc > 4)) {
        help();
    } else if (argc == 4) {
        debug = (strcmp(argv[3], "debug") == 0);
        multi = (strcmp(argv[3], "multi") == 0);
    }

    if (strcmp(argv[1], "start") == 0) {
        int port = strtol(argv[2], NULL, 10);
        int inputPid = 0;
        if (!debug && !multi && argc == 4) {
            inputPid = strtol(argv[3], NULL, 10);
        }
        ret = WxInitSDK(debug, multi, port, inputPid);
    } else if (strcmp(argv[1], "stop") == 0) {
        ret = WxDestroySDK();
    } else {
        help();
    }

    return ret;
}
