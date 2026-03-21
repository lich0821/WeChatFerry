package com.wechat.ferry.enums;

import java.util.Arrays;
import java.util.Map;
import java.util.stream.Collectors;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * 枚举-微信联系人类型
 * 
 * @author chandler
 * @date 2024/10/02 18:35
 */
@Getter
@AllArgsConstructor
public enum WxContactsTypeEnum {

    /**
     * 1-个微
     */
    PERSON("1", "个微", ""),

    /**
     * 2-企微
     */
    WORK("2", "企微", "@openim"),

    /**
     * 3-群组
     */
    GROUP("3", "群组", "@chatroom"),

    /**
     * 4-官方杂号
     */
    OFFICIAL_MIXED_NO("4", "官方杂号", null),

    /**
     * 5-公众号
     */
    OFFICIAL_ACCOUNT("5", "公众号", "gh_"),

    /**
     * 未匹配上
     */
    UN_MATCH("", null, null),

    // 结束
    ;

    private final String code;
    private final String name;
    private final String affix;

    /**
     * map集合 key：code val：枚举
     */
    public static final Map<String, WxContactsTypeEnum> codeMap =
        Arrays.stream(values()).collect(Collectors.toMap(WxContactsTypeEnum::getCode, v -> v));

    /**
     * 根据code获取枚举
     */
    public static WxContactsTypeEnum getCodeMap(String code) {
        return codeMap.getOrDefault(code, UN_MATCH);
    }

}
