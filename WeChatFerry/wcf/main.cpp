#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sdk.h"

void help()
{
    fprintf(stderr,
            "\nUsage: \n"
            "启动: wcf.exe start port [debug] [--gui]\n"
            "关闭: wcf.exe stop [--gui]\n"
            "port : 命令端口(1-65534)，消息端口为命令端口+1\n"
            "debug: 注入 debug 版 spy\n"
            "--gui: 出错时弹模态框（默认写 stderr，适合自动化）\n"
            "\n退出码: \n"
            "  0 成功\n"
            "  2 命令行参数错误\n"
            "  3 spy DLL 不存在\n"
            "  4 打开/启动微信失败\n"
            "  5 注入 spy 失败\n"
            "  6 初始化 spy(RPC 服务)失败\n"
            "  7 .wcf.lock 读写失败\n"
            "  8 停止时微信未运行\n"
            "  9 卸载 spy 失败\n"
            " 10 spy 已注入(重复 start)\n");
}

int main(int argc, char *argv[])
{
    int ret    = -1;
    bool debug = false;

    if (argc < 2) {
        help();
        return WX_ERR_USAGE;
    }

    // 先扫描可选开关，剩下的按位置解析
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "debug") == 0) {
            debug = true;
        } else if (strcmp(argv[i], "--gui") == 0) {
            WxSetGuiMode(true);
        }
    }

    if (strcmp(argv[1], "start") == 0) {
        if (argc < 3) {
            fprintf(stderr, "缺少端口参数\n");
            help();
            return WX_ERR_USAGE;
        }
        char *end = NULL;
        long port = strtol(argv[2], &end, 10);
        if (end == argv[2] || *end != '\0' || port < 1 || port > 65534) {
            fprintf(stderr, "端口非法: %s（应为 1-65534 的整数）\n", argv[2]);
            return WX_ERR_USAGE;
        }
        ret = WxInitSDK(debug, (int)port);
    } else if (strcmp(argv[1], "stop") == 0) {
        ret = WxDestroySDK();
    } else {
        help();
        return WX_ERR_USAGE;
    }

    return ret;
}
