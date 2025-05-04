package com.wechat.ferry.service.impl;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

import javax.annotation.Resource;

import org.springframework.stereotype.Service;
import org.springframework.util.CollectionUtils;
import org.springframework.util.ObjectUtils;

import com.alibaba.fastjson2.JSON;
import com.alibaba.fastjson2.JSONObject;
import com.wechat.ferry.config.WeChatFerryProperties;
import com.wechat.ferry.entity.dto.WxPpMsgDTO;
import com.wechat.ferry.service.WeChatMsgService;
import com.wechat.ferry.strategy.msg.receive.ReceiveMsgFactory;
import com.wechat.ferry.strategy.msg.receive.ReceiveMsgStrategy;
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
        if (weChatFerryProperties.getOpenMsgGroupSwitch() && !weChatFerryProperties.getOpenMsgGroups().isEmpty()) {
            Map<String, List<String>> openMsgGroupMap = new LinkedHashMap<>();
            String allFnNoStr = "";
            List<String> allFnNoList = new ArrayList<>();
            if (weChatFerryProperties.getOpenMsgGroups().containsKey("ALL")) {
                allFnNoStr = weChatFerryProperties.getOpenMsgGroups().get("ALL");
                // 分割字符串并去除空格及空元素
                allFnNoList = Arrays.stream(allFnNoStr.split(","))
                    // 去掉前后空格
                    .map(String::trim)
                    // 过滤掉空字符串
                    .filter(s -> !s.isEmpty())
                    // 去重
                    .distinct().collect(Collectors.toList());
                openMsgGroupMap.put("ALL", allFnNoList);
            }

            // 遍历
            for (String key : weChatFerryProperties.getOpenMsgGroups().keySet()) {
                List<String> valList = new ArrayList<>();
                if (!"ALL".equals(key)) {
                    String str = weChatFerryProperties.getOpenMsgGroups().get(key);
                    String[] arr = str.split(",");
                    for (String s : arr) {
                        // 去重，且ALL中不包含
                        if (!valList.contains(s) && !allFnNoList.contains(s)) {
                            valList.add(s);
                        }
                    }
                    openMsgGroupMap.put(key, valList);
                }
            }

            // 指定处理的群聊
            if (!openMsgGroupMap.isEmpty()) {
                log.debug("[收到消息后处理]-[汇总后的所有功能]-openMsgGroupMap：{}", openMsgGroupMap);
                List<String> fnNoList = new ArrayList<>();
                // 先执行所有群都需要执行的
                if (openMsgGroupMap.containsKey("ALL")) {
                    fnNoList = openMsgGroupMap.get("ALL");
                }
                // 加入个性化的
                if (openMsgGroupMap.containsKey(dto.getRoomId())) {
                    fnNoList.addAll(openMsgGroupMap.get(dto.getRoomId()));
                }
                // 需要执行的策略
                if (!CollectionUtils.isEmpty(fnNoList)) {
                    log.debug("[收到消息后处理]-[汇总后的单群功能]-fnNoList：{}，群号：{}", fnNoList, dto.getRoomId());
                    for (String no : fnNoList) {
                        // 根据功能号获取对应的策略
                        ReceiveMsgStrategy receiveMsgStrategy = ReceiveMsgFactory.getStrategy(no);
                        receiveMsgStrategy.doHandle(dto);
                    }
                }
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
