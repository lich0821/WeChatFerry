package com.wechat.ferry.entity.vo.request;

import javax.validation.constraints.NotBlank;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求入参-查询-个微WCF数据库SQL查询
 *
 * @author chandler
 * @date 2024-10-02 17:10
 */
@Data
@ApiModel(value = "wxPpWcfDatabaseSqlReq", description = "个微WCF数据库SQL查询请求入参")
public class WxPpWcfDatabaseSqlReq {

    /**
     * 数据库名称
     */
    @NotBlank(message = "数据库名称不能为空")
    @ApiModelProperty(value = "数据库名称")
    private String databaseName;

    /**
     * SQL语句
     */
    @NotBlank(message = "SQL语句不能为空")
    @ApiModelProperty(value = "SQL语句")
    private String sqlText;

}
