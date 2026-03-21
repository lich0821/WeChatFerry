package com.wechat.ferry.enums;

import java.util.Arrays;
import java.util.Map;
import java.util.stream.Collectors;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * 枚举-性别
 * 
 * @author chandler
 * @date 2024/10/01 15:42
 */
@Getter
@AllArgsConstructor
public enum SexEnum {

    /**
     * 0-未知
     */
    UNKNOWN("0", "未知"),

    /**
     * 1-男
     */
    BOY("1", "男"),

    /**
     * 2-女
     */
    GIRL("2", "女"),

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
    public static final Map<String, SexEnum> codeMap = Arrays.stream(values()).collect(Collectors.toMap(SexEnum::getCode, v -> v));

    /**
     * 根据code获取枚举
     */
    public static SexEnum getCodeMap(String code) {
        return codeMap.getOrDefault(code, UN_MATCH);
    }

}
