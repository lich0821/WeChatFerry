package com.wechat.ferry.entity.vo.response;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求出参-个微WCF联系人
 *
 * @author chandler
 * @date 2024/10/02 17:01
 */
@Data
@ApiModel(value = "wxPpWcfContactsResp", description = "个微WCF联系人查询请求出参")
public class WxPpWcfContactsResp {

    /**
     * 微信内部识别号UID
     * 原始微信账号ID，以"wxid_"开头，初始默认的微信ID=微信号。
     */
    @ApiModelProperty(value = "微信内部识别号UID")
    private String weChatUid;

    /**
     * 微信号
     */
    @ApiModelProperty(value = "微信号")
    private String weChatNo;

    /**
     * 微信昵称
     */
    @ApiModelProperty(value = "微信昵称")
    private String weChatNickname;

    /**
     * 联系人备注
     */
    @ApiModelProperty(value = "联系人备注")
    private String weChatRemark;

    /**
     * 国家
     */
    @ApiModelProperty(value = "国家")
    private String country;

    /**
     * 国家拼音
     */
    @ApiModelProperty(value = "国家拼音")
    private String countryPinyin;

    /**
     * 省/州
     */
    @ApiModelProperty(value = "省/州")
    private String province;

    /**
     * 省/州拼音
     */
    @ApiModelProperty(value = "省/州拼音")
    private String provincePinyin;

    /**
     * 城市
     */
    @ApiModelProperty(value = "城市")
    private String city;

    /**
     * 城市拼音
     */
    @ApiModelProperty(value = "城市拼音")
    private String cityPinyin;

    /**
     * 性别
     */
    @ApiModelProperty(value = "性别")
    private String sex;

    /**
     * 性别-翻译
     */
    @ApiModelProperty(value = "性别-翻译")
    private String sexLabel;

    /**
     * 类型
     */
    @ApiModelProperty(value = "类型")
    private String type;

    /**
     * 类型-翻译
     */
    @ApiModelProperty(value = "类型-翻译")
    private String typeLabel;

}
