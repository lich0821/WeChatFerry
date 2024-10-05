package com.wechat.ferry.entity.vo.response;

import java.util.List;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求出参-个微数据库记录
 *
 * @author chandler
 * @date 2024/10/02 17:14
 */
@Data
@ApiModel(value = "wxPpDatabaseRowResp", description = "个微数据库记录查询请求出参")
public class WxPpDatabaseRowResp {

    /**
     * 字段列表
     */
    @ApiModelProperty(value = "字段列表")
    private List<WxPpDatabaseFieldResp> fieldList;

}
