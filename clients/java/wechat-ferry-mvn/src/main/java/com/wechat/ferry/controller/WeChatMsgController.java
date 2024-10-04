package com.wechat.ferry.controller;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import com.wechat.ferry.entity.TResponse;
import com.wechat.ferry.enums.ResponseCodeEnum;
import com.wechat.ferry.service.WeChatMsgService;

import io.swagger.annotations.Api;
import io.swagger.annotations.ApiOperation;
import lombok.extern.slf4j.Slf4j;

/**
 * 控制层-微信消息处理
 *
 * @author chandler
 * @date 2024-10-01 14:25
 */
@Slf4j
@RestController
@RequestMapping("/wechat/msg")
@Api(tags = "微信消息处理-接口")
public class WeChatMsgController {

    private WeChatMsgService weChatMsgService;

    @Autowired
    public void setWeChatMsgService(WeChatMsgService weChatMsgService) {
        this.weChatMsgService = weChatMsgService;
    }

    @ApiOperation(value = "接收微信消息", notes = "receiveMsg")
    @PostMapping(value = "/receive")
    public TResponse<Object> receiveMsg(@RequestBody String jsonString) {
        weChatMsgService.receiveMsg(jsonString);
        return TResponse.ok(ResponseCodeEnum.SUCCESS);
    }

}
