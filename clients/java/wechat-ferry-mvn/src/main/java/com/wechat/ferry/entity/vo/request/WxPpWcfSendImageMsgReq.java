package com.wechat.ferry.entity.vo.request;

import javax.validation.constraints.NotBlank;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求入参-个微WCF发送图片消息
 *
 * @author chandler
 * @date 2024-10-04 15:55
 */
@Data
@ApiModel(value = "wxPpWcfSendImageMsgReq", description = "个微WCF发送图片消息请求入参")
public class WxPpWcfSendImageMsgReq {

    /**
     * 资源路径-本地图片地址
     * 需要确保图片路径正确，建议使用绝对路径（使用双斜杠\\）
     * 如：`C:/Projs/WeChatRobot/TEQuant.jpeg`
     * 或 `https://raw.githubusercontent.com/lich0821/WeChatFerry/master/assets/TEQuant.jpg`
     */
    @NotBlank(message = "资源路径不能为空")
    @ApiModelProperty(value = "资源路径-本地图片地址")
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
