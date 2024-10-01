package com.wechat.ferry.service.impl;

import org.springframework.stereotype.Service;

import com.alibaba.fastjson2.JSON;
import com.wechat.ferry.entity.dto.WxMsgDTO;
import com.wechat.ferry.service.WeChatMsgService;

import lombok.extern.slf4j.Slf4j;

/**
 * 业务实现层-消息处理
 *
 * @author chandler
 * @date 2024-10-01 14:35
 */
@Slf4j
@Service
public class WeChatMsgServiceImpl implements WeChatMsgService {

    @Override
    public void receiveMsg(String jsonString) {
        // 转为JSON对象
        WxMsgDTO dto = JSON.parseObject(jsonString, WxMsgDTO.class);
        // TODO 这里可以拓展自己需要的功能
        log.debug("[收到消息]-[消息内容]-打印：{}", dto);
    }

}
