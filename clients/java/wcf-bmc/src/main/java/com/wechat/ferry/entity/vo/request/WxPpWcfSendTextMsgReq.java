package com.wechat.ferry.entity.vo.request;

import java.util.List;

import javax.validation.constraints.NotBlank;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * 请求入参-个微WCF发送文本消息
 *
 * @author chandler
 * @date 2024-10-02 20:33
 */
@Data
@ApiModel(value = "wxPpWcfSendTextMsgReq", description = "个微WCF发送文本消息请求入参")
public class WxPpWcfSendTextMsgReq {

    /**
     * 消息接收人
     * 消息接收人，私聊为 wxid（wxid_xxxxxxxxxxxxxx）
     * 群聊为 roomid（xxxxxxxxxx@chatroom）
     */
    @NotBlank(message = "消息接收人不能为空")
    @ApiModelProperty(value = "消息接收人")
    private String recipient;

    /**
     * 消息文本
     * 消息内容（如果是 @ 消息则需要有跟 @ 的人数量相同的 @）
     * 换行使用 `\\\\n` （单杠）
     */
    @NotBlank(message = "消息文本不能为空")
    @ApiModelProperty(value = "消息文本")
    private String msgText;

    /**
     * 要艾特的用户
     * 群聊时要 @ 的人（私聊时为空字符串），多个用逗号分隔。
     * 艾特所有人用 notify@all（必须是群主或者管理员才有权限）
     */
    @ApiModelProperty(value = "要艾特的用户")
    private List<String> atUsers;

    /**
     * 是否艾特全体
     * 默认为false
     */
    @ApiModelProperty(value = "是否艾特全体")
    private Boolean isAtAll = false;

}
