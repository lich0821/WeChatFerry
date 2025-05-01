package com.wechat.ferry.entity.vo.response;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求出参-个微WCF数据库字段
 *
 * @author chandler
 * @date 2024/10/02 17:15
 */
@Data
@ApiModel(value = "wxPpWcfDatabaseFieldResp", description = "个微WCF数据库字段查询请求出参")
public class WxPpWcfDatabaseFieldResp {

    /**
     * 字段类型
     */
    @ApiModelProperty(value = "字段类型")
    private String type;

    /**
     * 字段
     */
    @ApiModelProperty(value = "字段")
    private String column;

    /**
     * 字段值
     */
    @ApiModelProperty(value = "字段值")
    private Object value;

}
