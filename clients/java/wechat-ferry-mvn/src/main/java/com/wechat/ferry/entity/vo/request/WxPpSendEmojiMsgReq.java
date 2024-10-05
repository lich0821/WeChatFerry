package com.wechat.ferry.entity.vo.request;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求入参-个微发送表情消息
 *
 * @author chandler
 * @date 2024-10-04 23:14
 */
@Data
@ApiModel(value = "wxPpSendEmojiMsgReq", description = "个微发送表情消息请求入参")
public class WxPpSendEmojiMsgReq {

    /**
     * 路径
     */
    @ApiModelProperty(value = "路径")
    private String path;

    /**
     * 消息接收人
     * 消息接收人，私聊为 wxid（wxid_xxxxxxxxxxxxxx）
     * 群聊为 roomid（xxxxxxxxxx@chatroom）
     */
    @ApiModelProperty(value = "消息接收人")
    private String recipient;

}
