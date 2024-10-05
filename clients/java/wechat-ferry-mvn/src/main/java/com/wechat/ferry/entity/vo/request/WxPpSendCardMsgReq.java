package com.wechat.ferry.entity.vo.request;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求入参-个微发送卡片消息
 *
 * @author chandler
 * @date 2024-10-04 23:11
 */
@Data
@ApiModel(value = "wxPpSendCardMsgReq", description = "个微发送卡片消息请求入参")
public class WxPpSendCardMsgReq {

    /**
     * 消息接收人
     * 消息接收人，私聊为 wxid（wxid_xxxxxxxxxxxxxx）
     * 群聊为 roomid（xxxxxxxxxx@chatroom）
     */
    @ApiModelProperty(value = "消息接收人")
    private String recipient;

    /**
     * XML报文
     */
    @ApiModelProperty(value = "XML报文")
    private String xml;

    /**
     * 路径
     */
    @ApiModelProperty(value = "路径")
    private String path;

    /**
     * 类型
     */
    @ApiModelProperty(value = "类型")
    private Integer type;

}
