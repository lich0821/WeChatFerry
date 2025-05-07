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
    private Long msgId;

    /**
     * 消息中的extra
     */
    @ApiModelProperty(value = "extra")
    private String extra;

    /**
     * 缩略图的链接
     */
    @ApiModelProperty(value = "缩略图的链接")
    private String thumbnailUrl;

    /**
     * 文件保存路径
     */
    @ApiModelProperty(value = "文件保存路径")
    private String savePath;

    /**
     * 文件类型后缀
     * 如：.png
     */
    @ApiModelProperty(value = "文件类型后缀")
    private String fileType;

    /**
     * 超时时间（秒）
     */
    @ApiModelProperty(value = "超时时间（秒）")
    private Integer timeout = 30;

}
