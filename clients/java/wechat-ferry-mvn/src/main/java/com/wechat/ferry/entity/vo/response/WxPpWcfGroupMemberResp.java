package com.wechat.ferry.entity.vo.response;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求出参-个微WCF查询群成员
 *
 * @author chandler
 * @date 2024/10/01 21:26
 */
@Data
@ApiModel(value = "wxPpWcfGroupMemberResp", description = "个微WCF群成员查询请求出参")
public class WxPpWcfGroupMemberResp {

    /**
     * 微信内部识别号UID
     * 原始微信账号ID，以"wxid_"开头，初始默认的微信ID=微信号。
     */
    @ApiModelProperty(value = "微信内部识别号UID")
    private String weChatUid;

    /**
     * 群内昵称
     */
    @ApiModelProperty(value = "群内昵称")
    private String groupNickName;

    /**
     * 状态
     */
    @ApiModelProperty(value = "状态")
    private String state;

    /**
     * 是否自己
     */
    @ApiModelProperty(value = "是否自己")
    private Boolean whetherSelf;

    /**
     * 是否企微
     */
    @ApiModelProperty(value = "是否企微")
    private Boolean whetherWork;

}
