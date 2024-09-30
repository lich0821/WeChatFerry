package com.iamteer.service.impl;

import java.util.List;

import org.springframework.stereotype.Service;

import com.iamteer.BussinessContext;
import com.iamteer.service.TestService;
import com.iamteer.utils.SpringContextHolderUtil;

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

    @Override
    public Boolean isLogin() {

        BussinessContext bussinessContext = SpringContextHolderUtil.getBean(BussinessContext.class);

        boolean flag = bussinessContext.getClient().isLogin();
        log.info("flag:{}", flag);
        List<String> list = bussinessContext.getClient().getDbNames();
        log.info("list:{}", list);
        return flag;
    }

}
