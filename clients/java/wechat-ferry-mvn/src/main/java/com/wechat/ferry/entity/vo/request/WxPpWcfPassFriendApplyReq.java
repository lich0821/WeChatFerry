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
     * 申请人
     * v3 xml.attrib["encryptusername"]
     */
    @NotBlank(message = "申请人不能为空")
    @ApiModelProperty(value = "申请人")
    private String applicant;

    /**
     * 审核人
     * v4 xml.attrib["ticket"]
     * 一般指自己，别人申请添加，自己审核是否通过
     */
    @NotBlank(message = "审核人不能为空")
    @ApiModelProperty(value = "审核人")
    private String reviewer;

    /**
     * 场景
     */
    @ApiModelProperty(value = "场景")
    private String scene;

}
