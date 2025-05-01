package com.wechat.ferry.entity.vo.request;

import javax.validation.constraints.NotBlank;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求入参-个微WCF发送文件消息
 *
 * @author chandler
 * @date 2024-10-04 23:08
 */
@Data
@ApiModel(value = "wxPpWcfSendFileMsgReq", description = "个微WCF发送文件消息请求入参")
public class WxPpWcfSendFileMsgReq {

    /**
     * 资源路径-本地文件路径
     */
    @NotBlank(message = "资源路径不能为空")
    @ApiModelProperty(value = "资源路径-本地文件路径")
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
