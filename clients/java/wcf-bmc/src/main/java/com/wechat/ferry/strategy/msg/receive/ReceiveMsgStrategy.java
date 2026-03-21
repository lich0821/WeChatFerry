package com.wechat.ferry.strategy.msg.receive;

import com.wechat.ferry.entity.dto.WxPpMsgDTO;

/**
 * 策略接口-消息处理-接收消息处理
 *
 * @author chandler
 * @date 2024-12-25 14:07
 */
public interface ReceiveMsgStrategy {

    /**
     * 获取策略的类型
     *
     * @return 返回代表策略类型的字符串
     */
    String getStrategyType();

    /**
     * 具体的处理
     * 
     * @return 如果是多字段，可以转为JSON字符串返回，已适配不同的返回数据
     */
    String doHandle(WxPpMsgDTO dto);

}
