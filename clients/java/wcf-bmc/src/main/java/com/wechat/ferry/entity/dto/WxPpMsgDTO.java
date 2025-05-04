package com.wechat.ferry.entity.dto;

import com.alibaba.fastjson2.JSONObject;
import com.fasterxml.jackson.annotation.JsonIgnore;

import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * DTO-个微信消息
 *
 * @author chandler
 * @date 2024-09-26 19:56
 */
@Data
public class WxPpMsgDTO {

    /**
     * 是否自己发送的
     */
    @ApiModelProperty(value = "是否自己发送的")
    private Boolean isSelf;

    /**
     * 是否群消息
     */
    @ApiModelProperty(value = "是否群消息")
    private Boolean isGroup;

    /**
     * 消息id
     */
    @ApiModelProperty(value = "消息id")
    private Long id;

    /**
     * 消息类型
     */
    @ApiModelProperty(value = "消息类型")
    private Integer type;

    /**
     * 消息类型
     */
    @ApiModelProperty(value = "消息类型")
    private Integer ts;

    /**
     * 群id（如果是群消息的话）
     */
    @ApiModelProperty(value = "群id（如果是群消息的话）")
    private String roomId;

    /**
     * 消息内容
     */
    @ApiModelProperty(value = "消息内容")
    private String content;

    /**
     * 转为JSON的内容
     * 对应DLL中的content原始内容
     */
    @ApiModelProperty(value = "转为JSON的内容")
    private JSONObject jsonContent;

    /**
     * 引用内容
     * 对应DLL中的content原始内容
     */
    @JsonIgnore
    @ApiModelProperty(value = "消息内容XML")
    private String quoteContent;

    /**
     * 消息发送者
     */
    @ApiModelProperty(value = "消息发送者")
    private String sender;

    /**
     * 签名
     */
    @ApiModelProperty(value = "签名")
    private String sign;

    /**
     * 缩略图
     */
    @ApiModelProperty(value = "缩略图")
    private String thumb;

    /**
     * 附加内容
     */
    @ApiModelProperty(value = "附加内容")
    private String extra;

    /**
     * 消息xml
     */
    @ApiModelProperty(value = "消息xml")
    private String xml;

}
