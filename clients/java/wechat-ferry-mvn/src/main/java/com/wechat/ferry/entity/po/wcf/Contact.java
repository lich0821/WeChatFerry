package com.wechat.ferry.entity.po.wcf;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 实体类-联系人表
 *
 * @author chandler
 * @date 2024-12-27 09:47
 */
@Data
@ApiModel(value = "Contact对象", description = "联系人表")
public class Contact {

    /**
     * 用户名
     */
    @ApiModelProperty(value = "用户名")
    private String userName;

    /**
     * 别名
     */
    @ApiModelProperty(value = "别名")
    private String alias;

    /**
     * 昵称
     */
    @ApiModelProperty(value = "昵称")
    private String nickname;

    /**
     * 删除标志
     */
    @ApiModelProperty(value = "删除标志")
    private Integer delFlag;

    /**
     * 验证标志
     */
    @ApiModelProperty(value = "验证标志")
    private Integer verifyFlag;

    /**
     * 备注
     */
    @ApiModelProperty(value = "备注")
    private String remark;

    /**
     * 标签ID列表
     */
    @ApiModelProperty(value = "标签ID列表")
    private String labelIdList;

    /**
     * 域名列表
     */
    @ApiModelProperty(value = "域名列表")
    private String domainList;

    /**
     * 群类型
     */
    @ApiModelProperty(value = "群类型")
    private Integer chatRoomType;

}
