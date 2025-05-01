package com.wechat.ferry.enums;

import java.util.Arrays;
import java.util.Map;
import java.util.stream.Collectors;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * 枚举-表名称
 * 
 * @author chandler
 * @date 2024/12/24 15:24
 */
@Getter
@AllArgsConstructor
public enum TableNameEnum {

    /**
     * CONTACT-联系人
     */
    CONTACT("Contact", "联系人", ""),

    /**
     * 未匹配上
     */
    UN_MATCH("", null, ""),

    // 结束
    ;

    private final String code;
    private final String name;
    private final String db;

    /**
     * map集合 key：code val：枚举
     */
    public static final Map<String, TableNameEnum> codeMap = Arrays.stream(values()).collect(Collectors.toMap(TableNameEnum::getCode, v -> v));

    /**
     * 根据code获取枚举
     */
    public static TableNameEnum getCodeMap(String code) {
        return codeMap.getOrDefault(code, UN_MATCH);
    }

}
