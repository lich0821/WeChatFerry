package com.wechat.ferry.entity.vo.request;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求入参-个微发送图片消息
 *
 * @author chandler
 * @date 2024-10-04 15:55
 */
@Data
@ApiModel(value = "wxPpSendImageMsgReq", description = "个微发送图片消息请求入参")
public class WxPpSendImageMsgReq {

    /**
     * 图片地址
     */
    @ApiModelProperty(value = "图片地址")
    private String path;

    /**
     * 消息接收人
     * 消息接收人，私聊为 wxid（wxid_xxxxxxxxxxxxxx）
     * 群聊为 roomid（xxxxxxxxxx@chatroom）
     */
    @ApiModelProperty(value = "消息接收人")
    private String recipient;

}
