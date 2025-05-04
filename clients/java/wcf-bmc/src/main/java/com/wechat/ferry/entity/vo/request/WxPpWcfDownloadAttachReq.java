package com.wechat.ferry.entity.vo.request;

import javax.validation.constraints.NotBlank;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

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
     * 消息编号
     */
    @NotBlank(message = "消息编号不能为空")
    @ApiModelProperty(value = "消息编号")
    private String msgId;

    /**
     * 文件的extra
     */
    @ApiModelProperty(value = "extra")
    private String extra;

    /**
     * 缩略图的链接
     */
    @ApiModelProperty(value = "缩略图的链接")
    private String thumbnailUrl;

    /**
     * 资源路径: 存放图片的目录。下载图片需要。暂不支持视频
     */
    @ApiModelProperty(value = "图片存放路径")
    private String resourcePath;

    /**
     * 文件类型后缀
     */
    @ApiModelProperty(value = "文件类型后缀")
    private String fileType;

}
