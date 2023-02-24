#include <stdio.h>
#include <string.h>

#include "framework.h"

#include "log.h"
#include "sdk.h"

void help() { LOG_INFO("Usage: wcf.exe start|stop [debug]\n"); }

int main(int argc, char *argv[])
{
    int ret    = -1;
    bool debug = false;

    if ((argc < 2) || (argc > 3)) {
        help();
    } else if (argc == 3) {
        debug = (strcmp(argv[2], "debug") == 0);
    }

    if (strcmp(argv[1], "start") == 0) {
        ret = WxInitSDK(debug);
    } else if (strcmp(argv[1], "stop") == 0) {
        ret = WxDestroySDK();
    } else {
        help();
    }

    return ret;
}