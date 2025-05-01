package com.wechat.ferry.entity.vo.request;

import javax.validation.constraints.NotBlank;

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
     * 需要确保图片路径正确，建议使用绝对路径（使用双斜杠\\）
     */
    @NotBlank(message = "资源路径不能为空")
    @ApiModelProperty(value = "资源路径-本地表情路径")
    private String resourcePath;

    /**
     * 消息接收人
     * 消息接收人，私聊为 wxid（wxid_xxxxxxxxxxxxxx）
     * 群聊为 roomid（xxxxxxxxxx@chatroom）
     */
    @NotBlank(message = "消息接收人不能为空")
    @ApiModelProperty(value = "消息接收人")
    private String recipient;

}
