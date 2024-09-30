package com.wechat.ferry.config;

import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.stereotype.Component;

import lombok.Data;

/**
 * 配置文件-WeChatFerry的配置文件
 *
 * @author chandler
 * @date 2024-09-21 21:35
 */
@Data
@Component
@ConfigurationProperties(prefix = "wechat.ferry")
public class WeChatFerryProperties {

    /**
     * dll文件位置
     */
    private String dllPath;

    /**
     * socket端口
     */
    private Integer socketPort;

}
