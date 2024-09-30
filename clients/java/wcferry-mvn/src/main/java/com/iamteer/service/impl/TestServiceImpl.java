package com.iamteer.service.impl;

import java.util.List;

import javax.annotation.Resource;

import org.springframework.stereotype.Service;

import com.iamteer.handle.WechatSocketClient;
import com.iamteer.service.TestService;

import lombok.extern.slf4j.Slf4j;

/**
 * 业务实现层-注册
 *
 * @author chandler
 * @date 2024-09-29 20:58
 */
@Slf4j
@Service
public class TestServiceImpl implements TestService {

    @Resource
    private WechatSocketClient wechatSocketClient;

    @Override
    public Boolean isLogin() {

        boolean flag = wechatSocketClient.isLogin();
        log.info("flag:{}", flag);
        List<String> list = wechatSocketClient.getDbNames();
        log.info("list:{}", list);
        return false;
    }

}
