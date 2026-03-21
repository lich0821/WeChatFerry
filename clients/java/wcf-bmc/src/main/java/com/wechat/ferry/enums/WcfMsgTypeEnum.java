package com.wechat.ferry.enums;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * 枚举-WCF消息类型
 * 
 * @author chandler
 * @date 2024/10/01 15:55
 */
@Getter
@AllArgsConstructor
public enum WcfMsgTypeEnum {

    /**
     * 0-朋友圈消息
     */
    FRIEND_CIRCLE_MSG("0", "朋友圈消息"),

    /**
     * 1-文字
     */
    TEXT("1", "文字"),

    /**
     * 3-图片
     */
    PICTURE("3", "图片"),

    /**
     * 34-语音
     */
    VOICE("34", "语音"),

    /**
     * 37-好友确认
     */
    FRIEND_CONFIRM("37", "好友确认"),

    /**
     * 40-可能认识的朋友消息-POSSIBLEFRIEND_MSG
     */
    POSSIBLE_FRIEND_MSG("40", "可能认识的朋友"),

    /**
     * 42-名片
     */
    VISITING_CARD("42", "名片"),

    /**
     * 43-视频
     */
    VIDEO("43", "视频"),

    /**
     * 47-石头剪刀布 | 表情图片
     */
    EMOJI_IMAGE("47", "石头剪刀布 | 表情图片"),

    /**
     * 48-位置
     */
    POSITION("48", "位置"),

    /**
     * 49-共享实时位置、文件、转账、链接
     */
    SHARE("49", "共享实时位置、文件、转账、链接"),

    /**
     * 50-语音电话消息-VOIPMSG
     */
    VOIP_MSG("50", "语音电话消息"),

    /**
     * 51-微信初始化
     */
    WECHAT_INIT("51", "微信初始化"),

    /**
     * 52-语音电话通知-VOIPNOTIFY
     */
    VOIP_NOTIFY("52", "语音电话通知"),

    /**
     * 53-语音电话邀请-VOIPINVITE
     */
    VOIP_INVITE("53", "语音电话邀请"),

    /**
     * 62-小视频
     */
    SMALL_VIDEO("62", "小视频"),

    /**
     * 66-微信红包
     */
    WECHAT_RED_ENVELOPE("66", "微信红包"),

    /**
     * 9999-系统通知-SYSNOTICE
     */
    SYS_NOTICE("9999", "系统通知"),

    /**
     * 10000-红包、系统消息
     */
    RED_ENVELOPE_SYS_NOTICE("10000", "红包、系统消息"),

    /**
     * 10002-撤回消息
     */
    WITHDRAW_MSG("10002", "撤回消息"),

    /**
     * 1048625-搜狗表情
     */
    SO_GOU_EMOJI("1048625", "搜狗表情"),

    /**
     * 16777265-链接
     */
    LINK("16777265", "链接"),

    /**
     * 436207665-微信红包
     */
    RED_ENVELOPE("436207665", "微信红包"),

    /**
     * 536936497-红包封面
     */
    RED_ENVELOPE_COVER("536936497", "红包封面"),

    /**
     * 754974769-视频号视频
     */
    VIDEO_NUMBER_VIDEO("754974769", "视频号视频"),

    /**
     * 771751985-视频号名片
     */
    VIDEO_NUMBER_CARD("771751985", "视频号名片"),

    /**
     * 822083633-引用消息
     */
    QUOTE_MSG("822083633", "引用消息"),

    /**
     * 922746929-拍一拍
     */
    PAT_ONE_PAT("922746929", "拍一拍"),

    /**
     * 973078577-视频号直播
     */
    VIDEO_NUMBER_LIVE("973078577", "视频号直播"),

    /**
     * 974127153-商品链接
     */
    PRODUCT_LINK("974127153", "商品链接"),

    /**
     * 975175729-视频号直播-TODO
     */
    UNKNOWN("975175729", "视频号直播"),

    /**
     * 1040187441-音乐链接
     */
    MUSIC_LINK("1040187441", "音乐链接"),

    /**
     * 1090519089-文件
     */
    FILE("1090519089", "文件"),

    /**
     * 未匹配上
     */
    UN_MATCH("", null),

    // 结束
    ;

    private final String code;
    private final String name;

}
