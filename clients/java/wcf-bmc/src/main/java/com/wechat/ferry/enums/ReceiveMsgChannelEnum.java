package com.wechat.ferry.enums;

import java.util.Arrays;
import java.util.Map;
import java.util.stream.Collectors;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * 枚举-接收消息处理渠道
 *
 * @author chandler
 * @date 2024/12/25 14:15
 */
@Getter
@AllArgsConstructor
public enum ReceiveMsgChannelEnum {

    /**
     * 1-签到
     */
    SIGN_IN("1", "签到"),

    /**
     * 未匹配上
     */
    UN_MATCH("", null),

    // END
    ;

    private final String code;
    private final String name;

    /**
     * map集合 key：code val：枚举
     */
    public static final Map<String, ReceiveMsgChannelEnum> codeMap =
        Arrays.stream(values()).collect(Collectors.toMap(ReceiveMsgChannelEnum::getCode, v -> v));

    /**
     * 根据code获取枚举
     */
    public static ReceiveMsgChannelEnum getCodeMap(String code) {
        return codeMap.getOrDefault(code, UN_MATCH);
    }

}
