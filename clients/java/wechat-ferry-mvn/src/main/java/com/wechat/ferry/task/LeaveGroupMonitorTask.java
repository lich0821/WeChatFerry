package com.wechat.ferry.task;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;
import org.springframework.util.CollectionUtils;

import com.wechat.ferry.entity.dto.WxPpUserDTO;
import com.wechat.ferry.entity.vo.request.WxPpWcfGroupMemberReq;
import com.wechat.ferry.entity.vo.response.WxPpWcfContactsResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfGroupMemberResp;
import com.wechat.ferry.enums.WxContactsTypeEnum;
import com.wechat.ferry.service.WeChatDllService;

import lombok.extern.slf4j.Slf4j;

@Slf4j
@Component
public class LeaveGroupMonitorTask {

    private WeChatDllService weChatDllService;

    @Autowired
    public void setWeChatDllService(WeChatDllService weChatDllService) {
        this.weChatDllService = weChatDllService;
    }

    /**
     * 群成员信息集合 key:群ID val:微信标识-weChatUid
     */
    private final Map<String, Map<String, String>> wxPpGroupMemberMap = new HashMap<>();

    @Scheduled(cron = "0 0/2 * * * ?")
    public void scheduled() {
        // 群变动
        List<String> groupList = new ArrayList<>();
        List<WxPpWcfContactsResp> contactsList = weChatDllService.queryContactsList();
        if (!CollectionUtils.isEmpty(contactsList)) {
            for (WxPpWcfContactsResp vo : contactsList) {
                if (WxContactsTypeEnum.GROUP.getCode().equals(vo.getType())) {
                    groupList.add(vo.getWeChatUid());
                }
            }
            // 清理我不在的群
            for (Map.Entry<String, Map<String, String>> entry : wxPpGroupMemberMap.entrySet()) {
                if (!groupList.contains(entry.getKey())) {
                    log.info("该账号自身退出了[{}]群组", entry.getKey());
                }
            }
        }

        // 群成员变动
        if (!CollectionUtils.isEmpty(groupList)) {
            WxPpWcfGroupMemberReq request;
            for (String gid : groupList) {
                request = new WxPpWcfGroupMemberReq();
                request.setGroupNo(gid);
                List<WxPpWcfGroupMemberResp> dbGroupMemberList = weChatDllService.queryGroupMemberList(request);
                if (!CollectionUtils.isEmpty(dbGroupMemberList)) {
                    // 现在的群成员
                    Map<String, String> nowGroupMemberMap = new HashMap<>();
                    for (WxPpWcfGroupMemberResp groupMember : dbGroupMemberList) {
                        nowGroupMemberMap.put(groupMember.getWeChatUid(), groupMember.getGroupNickName());
                    }

                    Map<String, String> oldGroupMemberMap = new HashMap<>();
                    // 判断之前有没有这个群
                    if (wxPpGroupMemberMap.containsKey(gid)) {
                        // 之前有这个群
                        oldGroupMemberMap = wxPpGroupMemberMap.get(gid);
                        // 遍历之前的群
                        for (Map.Entry<String, String> entry : oldGroupMemberMap.entrySet()) {
                            if (!nowGroupMemberMap.containsKey(entry.getKey())) {
                                log.info("{}-{}，这个人退出了[{}]群组", entry.getKey(), entry.getValue(), gid);
                            }
                        }
                    } else {
                        // 之前没这个群
                        wxPpGroupMemberMap.put(gid, nowGroupMemberMap);
                    }
                }
            }
        }
        log.info("[定时任务]-[重置签到]-结束");
    }

}
