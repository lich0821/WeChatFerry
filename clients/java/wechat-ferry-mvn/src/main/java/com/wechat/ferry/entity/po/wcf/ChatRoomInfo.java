package com.wechat.ferry.entity.po.wcf;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 实体类-群详细信息表
 *
 * @author chandler
 * @date 2024-12-27 10:03
 */
@Data
@ApiModel(value = "ChatRoomInfo", description = "群详细信息表")
public class ChatRoomInfo {

    /**
     * 群名称
     */
    @ApiModelProperty(value = "群名称")
    private String chatRoomName;

    /**
     * 群公告
     */
    @ApiModelProperty(value = "群公告")
    private String announcement;

    /**
     * 公告编辑者
     */
    @ApiModelProperty(value = "公告编辑者")
    private String announcementEditor;

    /**
     * 公告发布时间
     */
    @ApiModelProperty(value = "公告发布时间")
    private String announcementPublishTime;

    /**
     * 群状态
     */
    @ApiModelProperty(value = "群状态")
    private Integer chatRoomStatus;

}
