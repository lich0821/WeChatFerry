package com.wechat.ferry.enums;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * 枚举-是否
 * 
 * @author chandler
 * @date 2023/3/14 10:21
 */
@Getter
@AllArgsConstructor
public enum WhetherEnum {

    /**
     * 1-Y-是
     */
    YES("1", "是", "Y", true),

    /**
     * 2-N-否
     */
    NO("2", "否", "N", false),

    /**
     * 未匹配上
     */
    UN_MATCH("", null, null, null),

    // 结束
    ;

    private final String code;
    private final String name;
    private final String key;
    private final Boolean bool;

}
