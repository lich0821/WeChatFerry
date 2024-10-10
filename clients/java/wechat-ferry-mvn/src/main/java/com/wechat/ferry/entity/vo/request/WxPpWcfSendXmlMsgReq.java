package com.wechat.ferry.entity.vo.request;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求入参-个微WCF发送XML消息
 *
 * @author chandler
 * @date 2024-10-04 23:11
 */
@Data
@ApiModel(value = "wxPpWcfSendXmlMsgReq", description = "个微WCF发送XML消息请求入参")
public class WxPpWcfSendXmlMsgReq {

    /**
     * 消息接收人
     * 消息接收人，私聊为 wxid（wxid_xxxxxxxxxxxxxx）
     * 群聊为 roomid（xxxxxxxxxx@chatroom）
     */
    @ApiModelProperty(value = "消息接收人")
    private String recipient;

    /**
     * XML报文内容
     */
    @ApiModelProperty(value = "XML报文内容")
    private String xmlContent;

    /**
     * 资源路径-封面图片路径
     */
    @ApiModelProperty(value = "资源路径-封面图片路径")
    private String resourcePath;

    /**
     * xml类型，如：0x21 为小程序
     */
    @ApiModelProperty(value = "xml类型")
    private Integer xmlType;

}
