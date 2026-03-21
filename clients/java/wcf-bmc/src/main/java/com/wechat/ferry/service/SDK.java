package com.wechat.ferry.service;

import com.sun.jna.Library;

/**
 * SDK.dll的接口类
 *
 * @author xinggq
 * @date 2024-07-10 15:21
 */
public interface SDK extends Library {

    /**
     * 初始化SDK
     *
     * @param debug 开发模式
     * @param port 端口
     * @return 状态值
     */
    int WxInitSDK(boolean debug, int port);

    /**
     * 退出SDK
     *
     * @return 状态值
     */
    int WxDestroySDK();

}