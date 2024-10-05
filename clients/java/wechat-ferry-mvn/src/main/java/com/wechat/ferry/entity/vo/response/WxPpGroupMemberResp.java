package com.wechat.ferry.entity.vo.response;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求出参-个微查询群成员
 *
 * @author chandler
 * @date 2024/10/01 21:26
 */
@Data
@ApiModel(value = "wxPpGroupMemberResp", description = "个微群成员查询请求出参")
public class WxPpGroupMemberResp {

    /**
     * 微信编号
     */
    @ApiModelProperty(value = "微信编号")
    private String weChatNo;

    /**
     * 微信昵称
     */
    @ApiModelProperty(value = "微信昵称")
    private String nickName;

    /**
     * 状态
     */
    @ApiModelProperty(value = "状态")
    private String state;

}
