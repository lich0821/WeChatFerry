package com.wechat.ferry.strategy.msg.receive.impl;

import com.wechat.ferry.entity.dto.WxPpMsgDTO;
import com.wechat.ferry.enums.ReceiveMsgChannelEnum;
import com.wechat.ferry.strategy.msg.receive.ReceiveMsgStrategy;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Component;

/**
 * 策略实现类-接收消息-签到处理
 *
 * @author chandler
 * @date 2024-12-25 14:19
 */
@Slf4j
@Component
public class SignInMsgStrategyImpl implements ReceiveMsgStrategy {

    @Override
    public String getStrategyType() {
        log.debug("[接收消息]-[签到处理]-匹配到：{}-{}-策略", ReceiveMsgChannelEnum.SIGN_IN.getCode(), ReceiveMsgChannelEnum.SIGN_IN.getName());
        return ReceiveMsgChannelEnum.SIGN_IN.getCode();
    }

    @Override
    public String doHandle(WxPpMsgDTO dto) {
        // TODO 这里写具体的操作
        // 当前是使用的所有策略类全部执行 所以这里需要控制哪种类型才处理
        log.info("签到处理");
        return "";
    }

}
