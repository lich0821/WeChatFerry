package com.wechat.ferry.entity.vo.request;

import javax.validation.constraints.NotBlank;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求入参-文件保存
 *
 * @author wmz
 * @date 2025-05-02
 */
@Data
@ApiModel(value = "wxPpWcfFileSaveReq", description = "个微WCF文件保存请求入参")
public class WxPpWcfFileSaveReq {

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
