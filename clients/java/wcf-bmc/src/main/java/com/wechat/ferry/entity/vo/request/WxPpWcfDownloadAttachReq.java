package com.wechat.ferry.entity.vo.request;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

import javax.validation.constraints.NotBlank;

/**
 * 请求入参-下载附件信息
 *
 * @author wmz
 * @date 2025-05-02
 */
@Data
@ApiModel(value = "wxPpWcfDownloadAttachReq", description = "个微WCF下载附件请求入参")
public class WxPpWcfDownloadAttachReq {

    /**
     * 消息接收人
     * 消息接收人，私聊为 wxid（wxid_xxxxxxxxxxxxxx）
     * 群聊为 roomid（xxxxxxxxxx@chatroom）
     */
    @NotBlank(message = "消息id不能为空")
    @ApiModelProperty(value = "消息id")
    private Long id;

    /**
     * 文件的extra
     */
    @ApiModelProperty(value = "extra")
    private String extra;

    /**
     * 缩略图thumb
     */
//    @NotBlank(message = "thumb不能为空")
    @ApiModelProperty(value = "缩略图thumb")
    private String thumb;
    
    /**
     * dir (str): 存放图片的目录。下载图片需要。暂不支持视频
     */
    @ApiModelProperty(value = "图片存放路径dir")
    private String dir;

}
