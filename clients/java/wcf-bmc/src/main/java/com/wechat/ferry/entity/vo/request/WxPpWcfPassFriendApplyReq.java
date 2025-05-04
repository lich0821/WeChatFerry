package com.wechat.ferry.entity.vo.request;

import javax.validation.constraints.NotBlank;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求入参-通过好友申请
 *
 * @author chandler
 * @date 2024-12-25 09:30
 */
@Data
@ApiModel(value = "wxPpWcfPassFriendApplyReq", description = "个微WCF通过好友申请请求入参")
public class WxPpWcfPassFriendApplyReq {

    /**
     * 加密用户名
     * v3 xml.attrib["encryptusername"]
     * 加密用户名 (好友申请消息里 v3 开头的字符串)
     */
    @NotBlank(message = "加密用户名不能为空")
    @ApiModelProperty(value = "加密用户名")
    private String encryptUsername;

    /**
     * ticket
     * v4 xml.attrib["ticket"]
     * Ticket (好友申请消息里 v4 开头的字符串)
     */
    @NotBlank(message = "ticket不能为空")
    @ApiModelProperty(value = "ticket")
    private String ticket;

    /**
     * 场景
     * 申请方式 (好友申请消息里的 scene); 为了兼容旧接口，默认为扫码添加 (30)
     */
    @ApiModelProperty(value = "场景")
    private String scene;

}
