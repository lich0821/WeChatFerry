package com.wechat.ferry.enums;

import java.util.Arrays;
import java.util.Map;
import java.util.stream.Collectors;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * 枚举-消息回调开关
 * 1-关闭 2-全部回调 3-发送成功才回调
 * 
 * @author chandler
 * @date 2024/10/01 15:42
 */
@Getter
@AllArgsConstructor
public enum MsgCallbackTypeEnum {

    /**
     * 1-关闭
     */
    CLOSE("1", "关闭"),

    /**
     * 2-全部回调
     */
    ALL("2", "全部回调"),

    /**
     * 3-发送成功才回调
     */
    SUCCESS("3", "发送成功才回调"),

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
    public static final Map<String, MsgCallbackTypeEnum> codeMap =
        Arrays.stream(values()).collect(Collectors.toMap(MsgCallbackTypeEnum::getCode, v -> v));

    /**
     * 根据code获取枚举
     */
    public static MsgCallbackTypeEnum getCodeMap(String code) {
        return codeMap.getOrDefault(code, UN_MATCH);
    }

}
