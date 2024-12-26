package com.wechat.ferry.enums;

import java.util.Arrays;
import java.util.Map;
import java.util.stream.Collectors;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * 枚举-数据库名称
 * 
 * @author chandler
 * @date 2024/12/24 15:10
 */
@Getter
@AllArgsConstructor
public enum DatabaseNameEnum {

    /**
     * MicroMsg-
     */
    MICRO_MSG("MicroMsg.db", ""),

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
    public static final Map<String, DatabaseNameEnum> codeMap = Arrays.stream(values()).collect(Collectors.toMap(DatabaseNameEnum::getCode, v -> v));

    /**
     * 根据code获取枚举
     */
    public static DatabaseNameEnum getCodeMap(String code) {
        return codeMap.getOrDefault(code, UN_MATCH);
    }

}
