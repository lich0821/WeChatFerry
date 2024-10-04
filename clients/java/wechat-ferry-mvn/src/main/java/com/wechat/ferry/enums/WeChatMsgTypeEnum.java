package com.wechat.ferry.enums;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * 枚举-消息类型
 * 
 * @author chandler
 * @date 2024/10/01 15:55
 */
@Getter
@AllArgsConstructor
public enum WeChatMsgTypeEnum {

    /**
     * 0-未知
     */
    UNKNOWN("0", "未知"),

    /**
     * 未匹配上
     */
    UN_MATCH("", null),

    // 结束
    ;

    private final String code;
    private final String name;

}
