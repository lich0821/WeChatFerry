package com.wechat.ferry.config;

import java.util.List;
import java.util.Map;

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
     * 联系人类型-官方杂号，禁止与其他分类重复(格式：代码|名称)
     * 使用时记得需要提取代码或者名称匹配
     */
    private List<String> contactsTypeMixed;

    /**
     * 联系人类型-公众号，禁止与其他分类重复(格式：代码|名称)
     * 使用时记得需要提取代码或者名称匹配
     */
    private List<String> contactsTypeOfficial;

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
     * 发送消息转发标识 1-关闭 2-全转发 3-发送成功才转发
     */
    private String sendMsgFwdFlag = "1";

    /**
     * 发送消息转发URL
     */
    private List<String> sendMsgFwdUrls;

    /**
     * 调用第三方服务客户端成功状态码
     */
    private Map<String, String> thirdPartyOkCodes;

}
