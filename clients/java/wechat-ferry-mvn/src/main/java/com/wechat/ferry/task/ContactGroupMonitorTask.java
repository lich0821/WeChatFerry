package com.wechat.ferry.task;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.annotation.Resource;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;
import org.springframework.util.CollectionUtils;

import com.wechat.ferry.aggregation.facade.ContactDo;
import com.wechat.ferry.config.WeChatFerryProperties;
import com.wechat.ferry.entity.po.wcf.Contact;
import com.wechat.ferry.enums.WxContactsTypeEnum;
import com.wechat.ferry.handle.WeChatSocketClient;

import lombok.extern.slf4j.Slf4j;

@Slf4j
@Component
public class ContactGroupMonitorTask {

    private WeChatSocketClient wechatSocketClient;

    @Autowired
    public void setWechatSocketClient(WeChatSocketClient wechatSocketClient) {
        this.wechatSocketClient = wechatSocketClient;
    }

    @Resource
    private WeChatFerryProperties weChatFerryProperties;

    /**
     * 全部联系人信息集合 key:微信标识-weChatUid val:联系人信息
     */
    private final Map<String, ContactDo> allContactDoMap = new HashMap<>();

    /**
     * 个微联系人信息集合 key:微信标识-weChatUid val:联系人信息
     */
    private final Map<String, String> ppContactDoMap = new HashMap<>();

    /**
     * 企微联系人信息集合 key:微信标识-weChatUid val:联系人信息
     */
    private final Map<String, String> cpContactDoMap = new HashMap<>();

    /**
     * 群组信息集合 key:群ID val:群名称
     */
    private final Map<String, String> groupMap = new HashMap<>();

    /**
     * 群成员信息集合 key:群ID val:微信标识-weChatUid
     */
    private final Map<String, Map<String, String>> groupMemberMap = new HashMap<>();

    /**
     * 初始化标志
     */
    private Boolean initFlag = false;

    @Scheduled(cron = "0 0/2 * * * ?")
    public void scheduled() {
        ContactDo contactDo = new ContactDo();
        // 查询联系人
        List<ContactDo> contactList = contactDo.queryContactListBySql(wechatSocketClient, weChatFerryProperties);
        // 调用联系人监控处理
        contactMonitor(contactList);
        initFlag = true;
        log.info("[定时任务]-[重置签到]-结束");
    }

    // 联系人监控
    private void contactMonitor(List<ContactDo> contactList) {
        // 新增个微联系人
        List<String> addPpContactList = new ArrayList<>();
        // 删除个微联系人
        List<String> deletePpContactList = new ArrayList<>();
        // 新增企微联系人
        List<String> addCpContactList = new ArrayList<>();
        // 删除企微联系人
        List<String> deleteCpContactList = new ArrayList<>();
        // 新增群组
        List<String> addGroupList = new ArrayList<>();
        // 退出群组
        List<String> deleteGroupList = new ArrayList<>();
        // 本次的联系人标识列表
        List<String> nowContactIdList = new ArrayList<>();

        // 开始匹配
        if (!CollectionUtils.isEmpty(contactList)) {
            for (ContactDo contactDo : contactList) {
                nowContactIdList.add(contactDo.getWeChatUid());
                if (!initFlag) {
                    // 首次初始化
                    allContactDoMap.put(contactDo.getWeChatUid(), contactDo);
                    if (WxContactsTypeEnum.PERSON.getCode().equals(contactDo.getContactType())) {
                        // 个微-初始化
                        ppContactDoMap.put(contactDo.getWeChatUid(), contactDo.getNickname());
                    } else if (WxContactsTypeEnum.WORK.getCode().equals(contactDo.getContactType())) {
                        // 企业微信-初始化
                        cpContactDoMap.put(contactDo.getWeChatUid(), contactDo.getNickname());
                    } else if (WxContactsTypeEnum.GROUP.getCode().equals(contactDo.getContactType())) {
                        // 群组-初始化
                        groupMap.put(contactDo.getWeChatUid(), contactDo.getNickname());
                    }
                } else {
                    // 非首次
                    if (!ppContactDoMap.containsKey(contactDo.getWeChatUid())) {
                        // 个微-新增
                        addPpContactList.add(contactDo.getWeChatUid());
                        ppContactDoMap.put(contactDo.getWeChatUid(), contactDo.getNickname());
                    } else if (!cpContactDoMap.containsKey(contactDo.getContactType())) {
                        // 企业微信-新增
                        addCpContactList.add(contactDo.getWeChatUid());
                        cpContactDoMap.put(contactDo.getWeChatUid(), contactDo.getNickname());
                    } else if (!groupMap.containsKey(contactDo.getContactType())) {
                        // 群组-新增
                        addGroupList.add(contactDo.getWeChatUid());
                        groupMap.put(contactDo.getWeChatUid(), contactDo.getNickname());
                    }
                }
            }

            // 初始化完成
            if (initFlag) {
                // 个微
                for (Map.Entry<String, String> entry : ppContactDoMap.entrySet()) {
                    if (!nowContactIdList.contains(entry.getKey())) {
                        // 个微-删除
                        deletePpContactList.add(entry.getKey());
                    }
                }
                // 企微
                for (Map.Entry<String, String> entry : cpContactDoMap.entrySet()) {
                    if (!nowContactIdList.contains(entry.getKey())) {
                        // 企微-删除
                        deleteCpContactList.add(entry.getKey());
                    }
                }
                // 群组
                for (Map.Entry<String, String> entry : groupMap.entrySet()) {
                    if (!nowContactIdList.contains(entry.getKey())) {
                        // 群组-删除
                        deleteGroupList.add(entry.getKey());
                        log.info("\"{}\"离开了群聊");
                    }
                }
            }
        }
        log.info("[定时任务]-[联系人监控]-个微新增：{}，个微删除：{}，企微新增：{}，企微删除：{}，群组新增：{}，群组删除：{}", addPpContactList, deletePpContactList, addCpContactList,
            deleteCpContactList, addGroupList, deleteGroupList);
    }

    // 监控群成员
    private void groupMemberMonitor() {
        if (!groupMap.isEmpty()) {
            List<String> groupIdList = new ArrayList<>(groupMap.keySet());
            for (String groupId : groupIdList) {
                // 查询
            }
        }
    }

}
