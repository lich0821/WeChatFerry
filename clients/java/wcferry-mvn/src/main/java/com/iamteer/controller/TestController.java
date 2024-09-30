package com.iamteer.controller;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import com.iamteer.entity.TResponse;
import com.iamteer.enums.ResponseCodeEnum;
import com.iamteer.service.TestService;

import io.swagger.annotations.Api;
import io.swagger.annotations.ApiOperation;

@RestController
@RequestMapping("/test")
@Api(tags = "测试-接口")
public class TestController {

    private TestService testService;

    @Autowired
    public void setTestService(TestService testService) {
        this.testService = testService;
    }

    @ApiOperation(value = "测试", notes = "login")
    @PostMapping(value = "/login")
    public TResponse<Object> login() {
        Boolean flag = testService.isLogin();
        return TResponse.ok(ResponseCodeEnum.SUCCESS, flag);
    }

}
