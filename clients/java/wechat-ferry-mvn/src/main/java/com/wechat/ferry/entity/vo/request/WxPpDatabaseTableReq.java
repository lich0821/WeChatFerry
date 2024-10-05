package com.wechat.ferry.entity.vo.request;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求入参-查询-个微数据库表查询
 *
 * @author chandler
 * @date 2024-10-02 17:55
 */
@Data
@ApiModel(value = "wxPpDatabaseTableReq", description = "个微数据库表查询请求入参")
public class WxPpDatabaseTableReq {

    /**
     * 数据库名称
     */
    @ApiModelProperty(value = "数据库名称")
    private String databaseName;

}
