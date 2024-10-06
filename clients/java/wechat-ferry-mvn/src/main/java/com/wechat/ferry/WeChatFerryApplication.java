package com.wechat.ferry;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.scheduling.annotation.EnableScheduling;

/**
 * 启动类
 * 
 * @author chandler
 * @date 2024-09-21 12:19
 */
@EnableScheduling
@SpringBootApplication
public class WeChatFerryApplication {

    public static void main(String[] args) {
        SpringApplication.run(WeChatFerryApplication.class, args);
    }

}
