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
     * 微信内部识别号UID
     * 原始微信账号ID，以"wxid_"开头，初始默认的微信ID=微信号。
     */
    @ApiModelProperty(value = "微信内部识别号UID")
    private String weChatUid;

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
