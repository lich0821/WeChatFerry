package com.wechat.ferry.entity.vo.response;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求出参-个微发送图像消息
 *
 * @author chandler
 * @date 2024/10/03 10:17
 */
@Data
@ApiModel(value = "wxPpSendImageMsgResp", description = "个微发送图像消息请求出参")
public class WxPpSendImageMsgResp {

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
