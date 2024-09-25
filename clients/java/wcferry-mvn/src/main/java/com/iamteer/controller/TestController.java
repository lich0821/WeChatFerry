package com.iamteer.controller;

import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import com.iamteer.entity.TResponse;
import com.iamteer.enums.ResponseCodeEnum;

import io.swagger.annotations.Api;
import io.swagger.annotations.ApiOperation;

@RestController
@RequestMapping("/test")
@Api(tags = "测试-接口")
public class TestController {

    @ApiOperation(value = "测试", notes = "index")
    @PostMapping(value = "/index")
    public TResponse<Object> index() {
        return TResponse.ok(ResponseCodeEnum.SUCCESS, "");
    }

}
