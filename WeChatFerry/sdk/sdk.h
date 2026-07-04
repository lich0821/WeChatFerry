#pragma once

// 语义化退出码：调用方(Python / wcf.exe)可据此给出精确错误提示，
// 而不必只判断"成功/失败"。新增码请同步 wcf 的 help() 文档。
enum WxStatus {
    WX_OK                     = 0, // 成功
    WX_ERR_USAGE              = 2, // 命令行参数错误(仅 wcf.exe)
    WX_ERR_DLL_NOT_FOUND      = 3, // spy DLL 不存在
    WX_ERR_OPEN_WECHAT        = 4, // 打开/启动微信失败
    WX_ERR_INJECT             = 5, // 注入 spy 失败
    WX_ERR_INIT_SPY           = 6, // 初始化 spy(RPC 服务)失败
    WX_ERR_LOCK               = 7, // .wcf.lock 读写失败
    WX_ERR_WECHAT_NOT_RUNNING = 8, // 停止时微信未运行
    WX_ERR_EJECT              = 9, // 卸载 spy 失败
    WX_ERR_ALREADY_INJECTED   = 10, // spy 已注入(重复 start)
};

int WxInitSDK(bool debug, int port);
int WxDestroySDK();
void WxSetGuiMode(bool enable); // 开启后错误改用模态弹框，默认写 stderr
