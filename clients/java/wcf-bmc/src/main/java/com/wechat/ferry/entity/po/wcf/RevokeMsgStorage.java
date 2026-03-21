package com.wechat.ferry.entity.po.wcf;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 实体类-撤回消息存储表
 *
 * @author chandler
 * @date 2024-12-27 10:08
 */
@Data
@ApiModel(value = "RevokeMsgStorage对象", description = "撤回消息存储表")
public class RevokeMsgStorage {

    /**
     * 创建时间
     */
    @ApiModelProperty(value = "创建时间")
    private Integer createTime;

    /**
     * 消息服务ID
     */
    @ApiModelProperty(value = "消息服务ID")
    private Integer msgSvrId;

    /**
     * 撤回服务ID
     */
    @ApiModelProperty(value = "撤回服务ID")
    private Integer revokeSvrId;

}
