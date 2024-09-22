package com.iamteer.config;

import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.stereotype.Component;

import lombok.Data;

/**
 * 配置文件-UAM模块的外部接口
 *
 * @author chandler
 * @date 2024-04-26 21:35
 */
@Data
@Component
@ConfigurationProperties(prefix = "wcferry")
public class WcferryProperties {

    /**
     * dll文件位置
     */
    private String dllPath;

    /**
     * 端口
     */
    private Integer socketPort;

}
