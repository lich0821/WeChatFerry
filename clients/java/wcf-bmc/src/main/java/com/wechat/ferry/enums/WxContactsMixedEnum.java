package com.wechat.ferry.enums;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.stream.Collectors;

import org.springframework.util.ObjectUtils;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * 枚举-微信联系人-官方杂号
 * 
 * @author chandler
 * @date 2024/12/24 16:01
 */
@Getter
@AllArgsConstructor
public enum WxContactsMixedEnum {

    /**
     * fmessage-朋友推荐消息
     */
    F_MESSAGE("fmessage", "朋友推荐消息"),

    /**
     * medianote-语音记事本
     */
    MEDIA_NOTE("medianote", "语音记事本"),

    /**
     * floatbottle-漂流瓶
     */
    FLOAT_BOTTLE("floatbottle", "漂流瓶"),

    /**
     * filehelper-文件传输助手
     */
    FILE_HELPER("filehelper", "文件传输助手"),

    /**
     * newsapp-新闻
     */
    NEWS_APP("newsapp", "新闻"),

    /**
     * newsapp-微信团队
     */
    WEI_XIN("weixin", "微信团队"),

    /**
     * 未匹配上
     */
    UN_MATCH("", null),

    // 结束
    ;

    private final String code;
    private final String name;

    /**
     * map集合 key：code val：枚举
     */
    public static final Map<String, WxContactsMixedEnum> codeMap =
        Arrays.stream(values()).collect(Collectors.toMap(WxContactsMixedEnum::getCode, v -> v));

    /**
     * 根据code获取枚举
     */
    public static WxContactsMixedEnum getCodeMap(String code) {
        return codeMap.getOrDefault(code, UN_MATCH);
    }

    /**
     * map集合 key：code val：名称
     */
    public static Map<String, String> toCodeNameMap() {
        Map<String, String> map = new HashMap<>();
        for (WxContactsMixedEnum val : WxContactsMixedEnum.values()) {
            if (!ObjectUtils.isEmpty(val.getCode())) {
                map.put(val.getCode(), val.getName());
            }
        }
        return map;
    }

}
