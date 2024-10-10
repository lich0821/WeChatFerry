package com.wechat.ferry.entity.vo.request;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求入参-个微WCF发送表情消息
 *
 * @author chandler
 * @date 2024-10-04 23:14
 */
@Data
@ApiModel(value = "wxPpWcfSendEmojiMsgReq", description = "个微WCF发送表情消息请求入参")
public class WxPpWcfSendEmojiMsgReq {

    /**
     * 资源路径-本地表情路径
     */
    @ApiModelProperty(value = "资源路径-本地表情路径")
    private String resourcePath;

    /**
     * 消息接收人
     * 消息接收人，私聊为 wxid（wxid_xxxxxxxxxxxxxx）
     * 群聊为 roomid（xxxxxxxxxx@chatroom）
     */
    @ApiModelProperty(value = "消息接收人")
    private String recipient;

}
