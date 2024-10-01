package com.wechat.ferry.service;

import com.sun.jna.Library;

/**
 * SDK.dll的接口类
 *
 * @Author xinggq
 * @Date 2024/7/10
 *
 * @author xinggq
 * @date 2024-07-10 15:21
 */
public interface SDK extends Library {

    /**
     * 初始化SDK
     * 
     * @param debug
     * @param port
     * @return
     */
    int WxInitSDK(boolean debug, int port);

    /**
     * 退出SDK
     * 
     * @return
     */
    int WxDestroySDK();

}