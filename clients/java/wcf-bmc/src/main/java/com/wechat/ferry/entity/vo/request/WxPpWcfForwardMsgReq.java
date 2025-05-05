package com.wechat.ferry.entity.vo.request;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

import javax.validation.constraints.NotBlank;
import javax.validation.constraints.NotNull;

/**
 * 转发微信消息入参
 *
 * @author wmz
 * @date 2025-05-05
 */
@Data
@ApiModel(value = "wxPpWcfForwardMsgReq", description = "转发微信消息入参")
public class WxPpWcfForwardMsgReq {

    /**
     * 消息id
     */
    @NotNull(message = "消息id不能为空")
    @ApiModelProperty(value = "消息id")
    private Long id;

    /**
     * 消息接收人 消息接收人，私聊为 wxid（wxid_xxxxxxxxxxxxxx） 群聊为 roomid（xxxxxxxxxx@chatroom）
     */
    @NotBlank(message = "消息接收人")
    @ApiModelProperty(value = "receiver")
    private String receiver;

}
