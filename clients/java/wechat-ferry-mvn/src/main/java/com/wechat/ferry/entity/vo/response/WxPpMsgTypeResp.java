package com.wechat.ferry.entity.vo.response;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求出参-个微消息类型
 *
 * @author chandler
 * @date 2024/10/01 21:26
 */
@Data
@ApiModel(value = "wxPpMsgTypeResp", description = "个微消息类型查询请求出参")
public class WxPpMsgTypeResp {

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
