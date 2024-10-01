package com.wechat.ferry.service.impl;

import java.util.List;

import javax.annotation.Resource;

import com.wechat.ferry.handle.WeChatSocketClient;
import org.springframework.stereotype.Service;

import com.wechat.ferry.service.TestService;

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
    private WeChatSocketClient wechatSocketClient;

    @Override
    public Boolean isLogin() {

        boolean flag = wechatSocketClient.isLogin();
        log.info("flag:{}", flag);
        List<String> list = wechatSocketClient.getDbNames();
        log.info("list:{}", list);
        return false;
    }

}
