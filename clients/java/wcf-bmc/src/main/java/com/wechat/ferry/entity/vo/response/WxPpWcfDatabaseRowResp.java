package com.wechat.ferry.entity.vo.response;

import java.util.List;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求出参-个微WCF数据库记录
 *
 * @author chandler
 * @date 2024/10/02 17:14
 */
@Data
@ApiModel(value = "wxPpWcfDatabaseRowResp", description = "个微WCF数据库记录查询请求出参")
public class WxPpWcfDatabaseRowResp {

    /**
     * 字段列表
     */
    @ApiModelProperty(value = "字段列表")
    private List<WxPpWcfDatabaseFieldResp> fieldList;

}
