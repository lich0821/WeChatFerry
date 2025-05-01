package com.wechat.ferry.enums;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * 枚举-本服务定义消息类型
 * 
 * @author chandler
 * @date 2024/10/01 15:55
 */
@Getter
@AllArgsConstructor
public enum WxPpMsgTypeEnum {

    /**
     * 0-未知
     */
    UNKNOWN("0", "未知", "文本"),

    /**
     * 1-文本
     */
    TEXT("1", "文本", "文本"),

    /**
     * 2-表情符号
     */
    EMOJI("2", "表情符号", "1"),

    /**
     * 3-表情包(表情图片)
     */
    EMOJI_IMAGE("3", "表情包(表情图片)", "47"),

    /**
     * 4-引用消息
     */
    QUOTE("4", "引用消息", "49"),

    /**
     * 未匹配上
     */
    UN_MATCH("", null, "文本"),

    // 结束
    ;

    private final String code;
    private final String name;
    private final String dllCode;

}
