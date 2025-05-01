package com.wechat.ferry.entity.po.wcf;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 实体类-联系人头像信息表
 *
 * @author chandler
 * @date 2024-12-27 09:59
 */
@Data
@ApiModel(value = "ContactHeadImgUrl对象", description = "联系人头像信息表")
public class ContactHeadImgUrl {

    /**
     * 用户名
     */
    @ApiModelProperty(value = "用户名")
    private String userName;

    /**
     * 小头像URL
     */
    @ApiModelProperty(value = "小头像URL")
    private String smallHeadIngUrl;

    /**
     * 大头像URL
     */
    @ApiModelProperty(value = "大头像URL")
    private String bigHeadIngUrl;

}
