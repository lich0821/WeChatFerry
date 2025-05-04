package com.wechat.ferry.entity.vo.response;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求出参-登录WCF个微信息
 *
 * @author chandler
 * @date 2024/10/05 22:53
 */
@Data
@ApiModel(value = "wxPpWcfLoginInfoResp", description = "登录WCF个微信息查询请求出参")
public class WxPpWcfLoginInfoResp {

    /**
     * 微信内部识别号UID
     * 原始微信账号ID，以"wxid_"开头，初始默认的微信ID=微信号。
     */
    @ApiModelProperty(value = "微信内部识别号UID")
    private String weChatUid;

    /**
     * 微信昵称
     */
    @ApiModelProperty(value = "微信昵称")
    private String weChatNickname;

    /**
     * 手机号
     */
    @ApiModelProperty(value = "手机号")
    private String phone;

    /**
     * 文件/图片等父路径
     */
    @ApiModelProperty(value = "文件/图片等父路径")
    private String homePath;

}
