package com.wechat.ferry.controller;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import com.alibaba.fastjson2.JSONObject;
import com.wechat.ferry.entity.TResponse;
import com.wechat.ferry.enums.ResponseCodeEnum;
import com.wechat.ferry.service.WeChatDllService;

import io.swagger.annotations.Api;
import io.swagger.annotations.ApiOperation;
import lombok.extern.slf4j.Slf4j;

/**
 * 控制层-微信DLL处理
 *
 * @author chandler
 * @date 2024-10-01 15:48
 */
@Slf4j
@RestController
@RequestMapping("/wechat/cgi/dll")
@Api(tags = "微信消息处理-接口")
public class WeChatDllController {

    private WeChatDllService weChatDllService;

    @Autowired
    public void setWeChatDllService(WeChatDllService weChatDllService) {
        this.weChatDllService = weChatDllService;
    }

    @ApiOperation(value = "测试", notes = "test")
    @PostMapping(value = "/test")
    public TResponse<Object> test(@RequestBody JSONObject jsonData) {

        return TResponse.ok(ResponseCodeEnum.SUCCESS);
    }

}
