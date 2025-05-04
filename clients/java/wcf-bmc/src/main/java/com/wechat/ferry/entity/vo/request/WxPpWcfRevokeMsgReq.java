package com.wechat.ferry.entity.vo.request;

import javax.validation.constraints.NotBlank;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求入参-撤回消息
 *
 * @author chandler
 * @date 2024-12-25 12:00
 */
@Data
@ApiModel(value = "wxPpWcfRevokeMsgReq", description = "个微WCF撤回消息请求入参")
public class WxPpWcfRevokeMsgReq {

    /**
     * 消息编号
     */
    @ApiModelProperty(value = "消息编号")
    private String msgId;

}
