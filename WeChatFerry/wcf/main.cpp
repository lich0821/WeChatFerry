#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sdk.h"

void help()
{
    printf("\nUsage: \n启动: wcf.exe start port [debug]\n关闭: wcf.exe stop\nport: 命令端口, 消息端口为命令端口+1\n");
}

int main(int argc, char *argv[])
{
    int ret    = -1;
    bool debug = false;

    if ((argc < 2) || (argc > 4)) {
        help();
    } else if (argc == 4) {
        debug = (strcmp(argv[3], "debug") == 0);
    }

    if (strcmp(argv[1], "start") == 0) {
        int port = strtol(argv[2], NULL, 10);

        ret = WxInitSDK(debug, port);
    } else if (strcmp(argv[1], "stop") == 0) {
        ret = WxDestroySDK();
    } else {
        help();
    }

    return ret;
}
