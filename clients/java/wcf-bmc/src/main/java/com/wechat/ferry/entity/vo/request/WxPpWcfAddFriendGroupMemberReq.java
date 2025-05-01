package com.wechat.ferry.entity.vo.request;

import javax.validation.constraints.NotBlank;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

import java.util.List;

/**
 * 请求入参-添加群成员为好友
 *
 * @author chandler
 * @date 2024-12-25 09:53
 */
@Data
@ApiModel(value = "wxPpWcfAddFriendGroupMemberReq", description = "个微WCF添加群成员为好友请求入参")
public class WxPpWcfAddFriendGroupMemberReq {

    /**
     * 群编号
     */
    @NotBlank(message = "群编号不能为空")
    @ApiModelProperty(value = "群编号")
    private String groupNo;

    /**
     * 待添加的群成员列表
     */
    @ApiModelProperty(value = "待添加的群成员列表")
    private List<String> groupMembers;

}
