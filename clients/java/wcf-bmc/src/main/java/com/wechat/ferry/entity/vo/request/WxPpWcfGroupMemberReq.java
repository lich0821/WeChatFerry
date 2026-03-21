package com.wechat.ferry.entity.vo.request;

import javax.validation.constraints.NotBlank;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求入参-个微WCF查询群成员
 *
 * @author chandler
 * @date 2024-10-02 20:55
 */
@Data
@ApiModel(value = "wxPpWcfGroupMemberReq", description = "个微WCF查询群成员请求入参")
public class WxPpWcfGroupMemberReq {

    /**
     * 群编号
     */
    @NotBlank(message = "群编号不能为空")
    @ApiModelProperty(value = "群编号")
    private String groupNo;

}
