package com.wechat.ferry.entity.vo.request;

import java.util.List;

import javax.validation.constraints.NotBlank;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求入参-个微WCF邀请群成员
 *
 * @author chandler
 * @date 2024-12-25 09:51
 */
@Data
@ApiModel(value = "wxPpWcfInviteGroupMemberReq", description = "个微WCF邀请群成员请求入参")
public class WxPpWcfInviteGroupMemberReq {

    /**
     * 群编号
     */
    @NotBlank(message = "群编号不能为空")
    @ApiModelProperty(value = "群编号")
    private String groupNo;

    /**
     * 待邀请的群成员列表
     */
    @ApiModelProperty(value = "待邀请的群成员列表")
    private List<String> groupMembers;

}
