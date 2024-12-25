package com.wechat.ferry.service.impl;

import java.util.Map;

import javax.annotation.Resource;

import org.springframework.stereotype.Service;
import org.springframework.util.CollectionUtils;
import org.springframework.util.ObjectUtils;

import com.alibaba.fastjson2.JSON;
import com.alibaba.fastjson2.JSONObject;
import com.wechat.ferry.config.WeChatFerryProperties;
import com.wechat.ferry.entity.dto.WxPpMsgDTO;
import com.wechat.ferry.service.WeChatMsgService;
import com.wechat.ferry.utils.HttpClientUtil;

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

    @Resource
    private WeChatFerryProperties weChatFerryProperties;

    @Override
    public void receiveMsg(String jsonString) {
        // 转发接口处理
        receiveMsgCallback(jsonString);
        // 转为JSON对象
        WxPpMsgDTO dto = JSON.parseObject(jsonString, WxPpMsgDTO.class);
        // 有开启的群聊配置
        if (!CollectionUtils.isEmpty(weChatFerryProperties.getOpenMsgGroups())) {
            // 指定处理的群聊
            if (weChatFerryProperties.getOpenMsgGroups().contains(dto.getRoomId())) {
                // TODO 这里可以拓展自己需要的功能
            }
        }
        log.debug("[收到消息]-[消息内容]-打印：{}", dto);
    }

    private void receiveMsgCallback(String jsonString) {
        // 开启回调，且回调地址不为空
        if (weChatFerryProperties.getReceiveMsgCallbackSwitch() && !CollectionUtils.isEmpty(weChatFerryProperties.getReceiveMsgCallbackUrls())) {
            for (String receiveMsgFwdUrl : weChatFerryProperties.getReceiveMsgCallbackUrls()) {
                if (!receiveMsgFwdUrl.startsWith("http")) {
                    continue;
                }
                try {
                    String responseStr = HttpClientUtil.doPostJson(receiveMsgFwdUrl, jsonString);
                    if (judgeSuccess(responseStr)) {
                        log.error("[接收消息]-消息回调至外部接口,获取响应状态失败！-URL：{}", receiveMsgFwdUrl);
                    }
                    log.debug("[接收消息]-[回调接收到的消息]-回调消息至：{}", receiveMsgFwdUrl);
                } catch (Exception e) {
                    log.error("[接收消息]-消息回调接口[{}]服务异常：", receiveMsgFwdUrl, e);
                }
            }
        }
    }

    private Boolean judgeSuccess(String responseStr) {
        // 默认为通过
        boolean passFlag = false;
        if (!ObjectUtils.isEmpty(responseStr)) {
            JSONObject jSONObject = JSONObject.parseObject(responseStr);
            if (!ObjectUtils.isEmpty(jSONObject) && !CollectionUtils.isEmpty(weChatFerryProperties.getThirdPartyOkCodes())) {
                Map<String, String> codeMap = weChatFerryProperties.getThirdPartyOkCodes();
                for (Map.Entry<String, String> entry : codeMap.entrySet()) {
                    if (!ObjectUtils.isEmpty(jSONObject.get(entry.getKey())) && jSONObject.get(entry.getKey()).equals(entry.getValue())) {
                        passFlag = true;
                        break;
                    }
                }
            }
        }
        return passFlag;
    }

}
