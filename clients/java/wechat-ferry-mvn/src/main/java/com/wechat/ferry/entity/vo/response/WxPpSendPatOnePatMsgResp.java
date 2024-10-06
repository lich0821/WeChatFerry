package com.wechat.ferry.entity.vo.response;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求出参-个微发送拍一拍消息
 *
 * @author chandler
 * @date 2024/10/06 15:52
 */
@Data
@ApiModel(value = "wxPpSendPatOnePatMsgResp", description = "个微发送拍一拍消息请求出参")
public class WxPpSendPatOnePatMsgResp {

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
