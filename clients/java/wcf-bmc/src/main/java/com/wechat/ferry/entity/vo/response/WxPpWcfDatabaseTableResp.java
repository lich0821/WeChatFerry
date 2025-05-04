package com.wechat.ferry.entity.vo.response;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求出参-个微WCF数据库表
 *
 * @author chandler
 * @date 2024/12/24 13:40
 */
@Data
@ApiModel(value = "wxPpWcfDatabaseRowResp", description = "个微WCF数据库表查询请求出参")
public class WxPpWcfDatabaseTableResp {

    /**
     * 表名
     */
    @ApiModelProperty(value = "表名")
    private String tableName;

    /**
     * SQL
     */
    @ApiModelProperty(value = "SQL")
    private String sql;

}
