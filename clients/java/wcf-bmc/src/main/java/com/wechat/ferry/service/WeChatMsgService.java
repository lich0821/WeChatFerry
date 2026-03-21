package com.wechat.ferry.service;

/**
 * 业务接口-消息处理
 *
 * @author chandler
 * @date 2024-10-01 14:30
 */
public interface WeChatMsgService {

    /**
     * 接收消息
     *
     * @param jsonString json数据
     *
     * @author chandler
     * @date 2024-10-01 14:33
     */
    void receiveMsg(String jsonString);

}
