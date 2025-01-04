package com.wechat.ferry.enums;

import java.util.Arrays;
import java.util.Map;
import java.util.stream.Collectors;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * 枚举-消息事件类型
 * 
 * @author chandler
 * @date 2024/12/27 18:09
 */
@Getter
@AllArgsConstructor
public enum MsgEventTypeEnum {

    /**
     * 1-注入成功
     */
    INJECT("1", "注入成功"),

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
    public static final Map<String, MsgEventTypeEnum> codeMap = Arrays.stream(values()).collect(Collectors.toMap(MsgEventTypeEnum::getCode, v -> v));

    /**
     * 根据code获取枚举
     */
    public static MsgEventTypeEnum getCodeMap(String code) {
        return codeMap.getOrDefault(code, UN_MATCH);
    }

}
