package com.wechat.ferry.entity.vo.request;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求入参-个微WCF接收转账
 *
 * @author chandler
 * @date 2024-12-25 13:46
 */
@Data
@ApiModel(value = "wxPpWcfReceiveTransferReq", description = "个微WCF接收转账请求入参")
public class WxPpWcfReceiveTransferReq {

    /**
     * 转账人
     */
    @ApiModelProperty(value = "转账人")
    private String weChatUid;

    /**
     * 转账编号 transferId
     */
    @ApiModelProperty(value = "转账编号")
    private String transferId;

    /**
     * 交易编号 Transaction id
     */
    @ApiModelProperty(value = "交易编号")
    private String transactionId;

}
