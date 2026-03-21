package com.wechat.ferry.entity.po.wcf;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 实体类-群信息表
 *
 * @author chandler
 * @date 2024-12-27 10:01
 */
@Data
@ApiModel(value = "ChatRoom", description = "群信息表")
public class ChatRoom {

    /**
     * 群名称
     */
    @ApiModelProperty(value = "群名称")
    private String chatRoomName;

    /**
     * 用户名称列表
     */
    @ApiModelProperty(value = "用户名称列表")
    private String userNameList;

    /**
     * 显示名称列表
     */
    @ApiModelProperty(value = "显示名称列表")
    private String displayNameList;

}
