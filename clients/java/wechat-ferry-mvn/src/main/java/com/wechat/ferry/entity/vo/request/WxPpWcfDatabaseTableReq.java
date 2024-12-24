package com.wechat.ferry.entity.vo.request;

import javax.validation.constraints.NotBlank;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求入参-查询-个微WCF数据库表查询
 *
 * @author chandler
 * @date 2024-10-02 17:55
 */
@Data
@ApiModel(value = "wxPpWcfDatabaseTableReq", description = "个微WCF数据库表查询请求入参")
public class WxPpWcfDatabaseTableReq {

    /**
     * 数据库名称
     */
    @NotBlank(message = "数据库名称不能为空")
    @ApiModelProperty(value = "数据库名称")
    private String databaseName;

}
