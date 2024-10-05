package com.wechat.ferry.config;

import java.util.List;

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

    /**
     * 需要开启消息处理的群
     */
    private List<String> openMsgGroups;

    /**
     * 接收消息转发开关
     */
    private Boolean receiveMsgFwdSwitch = false;

    /**
     * 接收消息转发URL
     */
    private List<String> receiveMsgFwdUrls;

    /**
     * 发送消息前转发开关
     */
    private Boolean sendMsgFrontFwdSwitch = false;

    /**
     * 发送消息前转发URL
     */
    private List<String> sendMsgFrontFwdUrls;

    /**
     * 发送消息后转发开关
     */
    private Boolean sendMsgBackFwdSwitch = false;

    /**
     * 发送消息后转发URL
     */
    private List<String> sendMsgBackFwdUrls;

}
