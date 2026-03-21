package com.wechat.ferry.entity.vo.response;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求出参-个微WCF发送富文本消息
 *
 * @author chandler
 * @date 2024/10/06 15:46
 */
@Data
@ApiModel(value = "wxPpWcfSendRichTextMsgResp", description = "个微WCF发送富文本消息请求出参")
public class WxPpWcfSendRichTextMsgResp {

    /**
     * 类型编号
     */
    @ApiModelProperty(value = "类型编号")
    private Integer id;

    /**
     * 类型名称
     */
    @ApiModelProperty(value = "类型名称")
    private String name;

}
