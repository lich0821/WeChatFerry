package com.wechat.ferry.entity.vo.request;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

import javax.validation.constraints.NotBlank;

/**
 * 请求入参-个微WCF发送富文本消息
 *
 * @author chandler
 * @date 2024-10-06 15:40
 */
@Data
@ApiModel(value = "wxPpWcfSendRichTextMsgReq", description = "个微WCF发送富文本消息请求入参")
public class WxPpWcfSendRichTextMsgReq {

    /**
     * 消息接收人
     * 消息接收人，私聊为 wxid（wxid_xxxxxxxxxxxxxx）
     * 群聊为 roomid（xxxxxxxxxx@chatroom）
     */
    @NotBlank(message = "消息接收人不能为空")
    @ApiModelProperty(value = "消息接收人")
    private String recipient;

    /**
     * 左下显示的名字
     */
    @ApiModelProperty(value = "左下显示的名字")
    private String name;

    /**
     * 填公众号id 可以显示对应的头像（gh_ 开头的）
     */
    @ApiModelProperty(value = "资源路径-封面图片路径")
    private String account;

    /**
     * 标题，最多两行
     */
    @ApiModelProperty(value = "标题，最多两行")
    private String title;

    /**
     * 摘要，三行
     */
    @ApiModelProperty(value = "摘要，三行")
    private String digest;

    /**
     * 点击后跳转的链接
     */
    @ApiModelProperty(value = "点击后跳转的链接")
    private String jumpUrl;

    /**
     * 缩略图的链接
     */
    @ApiModelProperty(value = "缩略图的链接")
    private String thumbnailUrl;

}
