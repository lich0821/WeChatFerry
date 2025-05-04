package com.wechat.ferry.service.impl;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.annotation.Resource;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.util.CollectionUtils;
import org.springframework.util.ObjectUtils;

import com.alibaba.fastjson2.JSON;
import com.alibaba.fastjson2.JSONObject;
import com.google.protobuf.InvalidProtocolBufferException;
import com.wechat.ferry.aggregation.facade.ContactDo;
import com.wechat.ferry.config.WeChatFerryProperties;
import com.wechat.ferry.entity.proto.Wcf;
import com.wechat.ferry.entity.vo.request.WxPpWcfAddFriendGroupMemberReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfDatabaseSqlReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfDatabaseTableReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfDeleteGroupMemberReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfGroupMemberReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfInviteGroupMemberReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfPassFriendApplyReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfPatOnePatMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfReceiveTransferReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfRevokeMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfSendEmojiMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfSendFileMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfSendImageMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfSendRichTextMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfSendTextMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfSendXmlMsgReq;
import com.wechat.ferry.entity.vo.response.WxPpWcfContactsResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfDatabaseFieldResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfDatabaseRowResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfDatabaseTableResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfGroupMemberResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfLoginInfoResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfMsgTypeResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfSendEmojiMsgResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfSendFileMsgResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfSendImageMsgResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfSendPatOnePatMsgResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfSendRichTextMsgResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfSendTextMsgResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfSendXmlMsgResp;
import com.wechat.ferry.enums.DatabaseNameEnum;
import com.wechat.ferry.enums.MsgCallbackTypeEnum;
import com.wechat.ferry.enums.SexEnum;
import com.wechat.ferry.enums.WxContactsTypeEnum;
import com.wechat.ferry.exception.BizException;
import com.wechat.ferry.handle.WeChatSocketClient;
import com.wechat.ferry.service.WeChatDllService;
import com.wechat.ferry.utils.HttpClientUtil;

import lombok.extern.slf4j.Slf4j;

/**
 * 业务实现层-对接原本DLL的接口
 *
 * @author chandler
 * @date 2024-10-01 15:58
 */
@Slf4j
@Service
public class WeChatDllServiceImpl implements WeChatDllService {

    private WeChatSocketClient wechatSocketClient;

    @Autowired
    public void setWechatSocketClient(WeChatSocketClient wechatSocketClient) {
        this.wechatSocketClient = wechatSocketClient;
    }

    @Resource
    private WeChatFerryProperties weChatFerryProperties;

    @Override
    public Boolean loginStatus() {
        // 公共校验
        checkClientStatus();
        return wechatSocketClient.isLogin();
    }

    @Override
    public String queryLoginWeChatUid() {
        // 公共校验
        checkClientStatus();
        // FUNC_GET_SELF_WXID_VALUE
        return wechatSocketClient.getSelfWxId();
    }

    @Override
    public WxPpWcfLoginInfoResp queryLoginWeChatInfo() {
        // 公共校验
        checkClientStatus();
        WxPpWcfLoginInfoResp resp = new WxPpWcfLoginInfoResp();
        // FUNC_GET_USER_INFO_VALUE
        Wcf.UserInfo userInfo = wechatSocketClient.getUserInfo();
        if (!ObjectUtils.isEmpty(userInfo)) {
            resp.setWeChatUid(userInfo.getWxid());
            resp.setWeChatNickname(userInfo.getName());
            resp.setPhone(userInfo.getMobile());
            resp.setHomePath(userInfo.getHome());
        }
        return resp;
    }

    @Override
    public List<WxPpWcfMsgTypeResp> queryMsgTypeList() {
        // 公共校验
        checkClientStatus();
        List<WxPpWcfMsgTypeResp> list = new ArrayList<>();
        // FUNC_GET_MSG_TYPES_VALUE
        Map<Integer, String> msgTypeMap = wechatSocketClient.getMsgTypes();
        if (!msgTypeMap.isEmpty()) {
            WxPpWcfMsgTypeResp resp;
            for (Map.Entry<Integer, String> entry : msgTypeMap.entrySet()) {
                resp = new WxPpWcfMsgTypeResp();
                resp.setId(entry.getKey());
                resp.setName(entry.getValue());
                list.add(resp);
            }
        }
        return list;
    }

    @Override
    public List<WxPpWcfContactsResp> queryContactsList() {
        // 公共校验
        checkClientStatus();
        List<WxPpWcfContactsResp> list = new ArrayList<>();
        // FUNC_GET_CONTACTS_VALUE
        List<Wcf.RpcContact> rpcContactList = wechatSocketClient.getContacts();
        if (!CollectionUtils.isEmpty(rpcContactList)) {
            for (Wcf.RpcContact rpcContact : rpcContactList) {
                WxPpWcfContactsResp vo = new WxPpWcfContactsResp();
                vo.setWeChatUid(rpcContact.getWxid());
                vo.setWeChatNo(rpcContact.getCode());
                vo.setWeChatRemark(rpcContact.getRemark());
                vo.setWeChatNickname(rpcContact.getName());
                if (!ObjectUtils.isEmpty(rpcContact.getCountry())) {
                    vo.setCountryPinyin(rpcContact.getCountry());
                    if ("CN".equals(rpcContact.getCountry())) {
                        vo.setCountry("中国");
                    }
                }
                if (!ObjectUtils.isEmpty(rpcContact.getProvince())) {
                    vo.setProvincePinyin(rpcContact.getProvince());
                    vo.setProvince(null);
                }
                if (!ObjectUtils.isEmpty(rpcContact.getCity())) {
                    vo.setCityPinyin(rpcContact.getCity());
                    vo.setCity(null);
                }
                // 性别处理
                if (!ObjectUtils.isEmpty(rpcContact.getGender())) {
                    vo.setSex(SexEnum.getCodeMap(String.valueOf(rpcContact.getGender())).getCode());
                    vo.setSexLabel(SexEnum.getCodeMap(String.valueOf(rpcContact.getGender())).getName());
                }
                // 微信类型
                if (!ObjectUtils.isEmpty(rpcContact.getWxid())) {
                    String type = ContactDo.convertContactType(rpcContact.getWxid(), weChatFerryProperties);
                    vo.setType(type);
                    vo.setTypeLabel(WxContactsTypeEnum.getCodeMap(rpcContact.getWxid()).getName());
                }
                list.add(vo);
            }
        }
        return list;
    }

    @Override
    public List<String> queryDbTableNameList() {
        // 公共校验
        checkClientStatus();
        // FUNC_GET_DB_NAMES_VALUE
        return wechatSocketClient.getDbNames();
    }

    @Override
    public List<WxPpWcfDatabaseTableResp> queryDbTableList(WxPpWcfDatabaseTableReq request) {
        // 公共校验
        checkClientStatus();
        List<WxPpWcfDatabaseTableResp> list = new ArrayList<>();
        // FUNC_GET_DB_TABLES_VALUE
        Map<String, String> tableMap = wechatSocketClient.getDbTables(request.getDatabaseName());
        if (!tableMap.isEmpty()) {
            WxPpWcfDatabaseTableResp resp;
            for (Map.Entry<String, String> entry : tableMap.entrySet()) {
                resp = new WxPpWcfDatabaseTableResp();
                resp.setTableName(entry.getKey());
                resp.setSql(entry.getValue());
                list.add(resp);
            }
        }
        return list;
    }

    @Override
    public List<WxPpWcfDatabaseTableResp> queryDbTableList(String databaseName) {
        WxPpWcfDatabaseTableReq request = new WxPpWcfDatabaseTableReq();
        request.setDatabaseName(databaseName);
        return this.queryDbTableList(request);
    }

    @Override
    public List<WxPpWcfDatabaseRowResp> execDbQuerySql(WxPpWcfDatabaseSqlReq request) {
        // 公共校验
        checkClientStatus();
        List<WxPpWcfDatabaseRowResp> list = new ArrayList<>();
        List<Wcf.DbRow> wcfList = wechatSocketClient.querySql(request.getDatabaseName(), request.getSqlText());
        if (!CollectionUtils.isEmpty(wcfList)) {
            for (Wcf.DbRow dbRow : wcfList) {
                WxPpWcfDatabaseRowResp rowVo = new WxPpWcfDatabaseRowResp();
                List<WxPpWcfDatabaseFieldResp> fieldVoList = new ArrayList<>();
                for (Wcf.DbField dbField : dbRow.getFieldsList()) {
                    WxPpWcfDatabaseFieldResp fieldVo = new WxPpWcfDatabaseFieldResp();
                    Object value;
                    if (ObjectUtils.isEmpty(dbField.getType())) {
                        log.warn("未知的SQL类型: {}", dbField.getType());
                        value = dbField.getContent().toByteArray();
                    } else {
                        value = wechatSocketClient.convertSqlVal(dbField.getType(), dbField.getContent());
                    }
                    fieldVo.setType(String.valueOf(dbField.getType()));
                    fieldVo.setColumn(dbField.getColumn());
                    fieldVo.setValue(value);
                    fieldVoList.add(fieldVo);
                }
                rowVo.setFieldList(fieldVoList);
                list.add(rowVo);
            }
        }
        return list;
    }

    @Override
    public List<WxPpWcfDatabaseRowResp> execDbQuerySql(String databaseName, String sqlText) {
        WxPpWcfDatabaseSqlReq request = new WxPpWcfDatabaseSqlReq();
        request.setDatabaseName(databaseName);
        request.setSqlText(sqlText);
        return this.execDbQuerySql(request);
    }

    @Override
    public WxPpWcfSendTextMsgResp sendTextMsg(WxPpWcfSendTextMsgReq request) {
        // 公共校验
        checkClientStatus();
        log.info("[发送消息]-[文本消息]-入参打印：{}", request);
        String atUser = "";
        if (request.getIsAtAll()) {
            // 艾特全体，仅管理员有效
            atUser = "@all";
        } else {
            // 处理艾特的人员
            if (!CollectionUtils.isEmpty(request.getAtUsers())) {
                atUser = String.join(",", request.getAtUsers());
            }
        }
        // FUNC_SEND_TXT_VALUE
        int state = wechatSocketClient.sendText(request.getMsgText(), request.getRecipient(), atUser);
        // 回调处理
        String stringJson = JSON.toJSONString(request);
        sendMsgCallback(stringJson, state);
        return null;
    }

    @Override
    public WxPpWcfSendTextMsgResp sendTextMsg(String recipient, String msgText, List<String> atUsers, Boolean isAtAll) {
        if (ObjectUtils.isEmpty(isAtAll)) {
            isAtAll = false;
        }
        WxPpWcfSendTextMsgReq request = new WxPpWcfSendTextMsgReq();
        request.setRecipient(recipient);
        request.setMsgText(msgText);
        request.setAtUsers(atUsers);
        request.setIsAtAll(isAtAll);
        return this.sendTextMsg(request);
    }

    @Override
    public WxPpWcfSendRichTextMsgResp sendRichTextMsg(WxPpWcfSendRichTextMsgReq request) {
        // 公共校验
        checkClientStatus();
        log.info("[发送消息]-[富文本消息]-入参打印：{}", request);
        // FUNC_SEND_RICH_TXT_VALUE
        int state = wechatSocketClient.sendRichText(request.getName(), request.getAccount(), request.getTitle(), request.getDigest(),
            request.getJumpUrl(), request.getThumbnailUrl(), request.getRecipient());
        // 回调处理
        String stringJson = JSON.toJSONString(request);
        sendMsgCallback(stringJson, state);
        return null;
    }

    @Override
    public WxPpWcfSendRichTextMsgResp sendRichTextMsg(String recipient, String name, String account, String title, String digest, String jumpUrl,
        String thumbnailUrl) {
        WxPpWcfSendRichTextMsgReq request = new WxPpWcfSendRichTextMsgReq();
        request.setRecipient(recipient);
        request.setName(name);
        request.setAccount(account);
        request.setTitle(title);
        request.setDigest(digest);
        request.setJumpUrl(jumpUrl);
        request.setThumbnailUrl(thumbnailUrl);
        return this.sendRichTextMsg(request);
    }

    @Override
    public WxPpWcfSendXmlMsgResp sendXmlMsg(WxPpWcfSendXmlMsgReq request) {
        // 公共校验
        checkClientStatus();
        log.info("[发送消息]-[XML消息]-入参打印：{}", request);
        int xmlType = 0x21;
        if ("21".equals(request.getXmlType())) {
            // 小程序
            xmlType = 0x21;
        } else {
            xmlType = Integer.parseInt(request.getXmlType());
        }
        // FUNC_SEND_XML_VALUE
        int state = wechatSocketClient.sendXml(request.getRecipient(), request.getXmlContent(), request.getResourcePath(), xmlType);
        // 回调处理
        String stringJson = JSON.toJSONString(request);
        sendMsgCallback(stringJson, state);
        return null;
    }

    @Override
    public WxPpWcfSendXmlMsgResp sendXmlMsg(String recipient, String xmlContent, String resourcePath, String xmlType) {
        WxPpWcfSendXmlMsgReq request = new WxPpWcfSendXmlMsgReq();
        request.setRecipient(recipient);
        request.setXmlContent(xmlContent);
        request.setResourcePath(resourcePath);
        request.setXmlType(xmlType);
        return this.sendXmlMsg(request);
    }

    @Override
    public WxPpWcfSendImageMsgResp sendImageMsg(WxPpWcfSendImageMsgReq request) {
        // 公共校验
        checkClientStatus();
        log.info("[发送消息]-[图片消息]-入参打印：{}", request);
        WxPpWcfSendImageMsgResp resp = new WxPpWcfSendImageMsgResp();
        // FUNC_SEND_IMG_VALUE
        int state = wechatSocketClient.sendImage(request.getResourcePath(), request.getRecipient());
        // 回调处理
        String stringJson = JSON.toJSONString(request);
        sendMsgCallback(stringJson, state);
        return null;
    }

    @Override
    public WxPpWcfSendImageMsgResp sendImageMsg(String recipient, String resourcePath) {
        WxPpWcfSendImageMsgReq request = new WxPpWcfSendImageMsgReq();
        request.setRecipient(recipient);
        request.setResourcePath(resourcePath);
        return this.sendImageMsg(request);
    }

    @Override
    public WxPpWcfSendEmojiMsgResp sendEmojiMsg(WxPpWcfSendEmojiMsgReq request) {
        // 公共校验
        checkClientStatus();
        log.info("[发送消息]-[表情消息]-入参打印：{}", request);
        // FUNC_SEND_EMOTION_VALUE
        int state = wechatSocketClient.sendEmotion(request.getResourcePath(), request.getRecipient());
        // 回调处理
        String stringJson = JSON.toJSONString(request);
        sendMsgCallback(stringJson, state);
        return null;
    }

    @Override
    public WxPpWcfSendEmojiMsgResp sendEmojiMsg(String recipient, String resourcePath) {
        WxPpWcfSendEmojiMsgReq request = new WxPpWcfSendEmojiMsgReq();
        request.setRecipient(recipient);
        request.setResourcePath(resourcePath);
        return this.sendEmojiMsg(request);
    }

    @Override
    public WxPpWcfSendFileMsgResp sendFileMsg(WxPpWcfSendFileMsgReq request) {
        // 公共校验
        checkClientStatus();
        log.info("[发送消息]-[文件消息]-入参打印：{}", request);
        // FUNC_SEND_FILE_VALUE
        int state = wechatSocketClient.sendFile(request.getResourcePath(), request.getRecipient());
        // 回调处理
        String stringJson = JSON.toJSONString(request);
        sendMsgCallback(stringJson, state);
        return null;
    }

    @Override
    public WxPpWcfSendFileMsgResp sendFileMsg(String recipient, String resourcePath) {
        WxPpWcfSendFileMsgReq request = new WxPpWcfSendFileMsgReq();
        request.setRecipient(recipient);
        request.setResourcePath(resourcePath);
        return this.sendFileMsg(request);
    }

    @Override
    public WxPpWcfSendPatOnePatMsgResp patOnePat(WxPpWcfPatOnePatMsgReq request) {
        // 公共校验
        checkClientStatus();
        log.info("[发送消息]-[拍一拍消息]-入参打印：{}", request);
        // FUNC_SEND_PAT_MSG_VALUE
        int state = wechatSocketClient.sendPatMsg(request.getRecipient(), request.getPatUser());
        // 回调处理
        String stringJson = JSON.toJSONString(request);
        sendMsgCallback(stringJson, state);
        return null;
    }

    @Override
    public WxPpWcfSendPatOnePatMsgResp patOnePat(String recipient, String patUser) {
        WxPpWcfPatOnePatMsgReq request = new WxPpWcfPatOnePatMsgReq();
        request.setRecipient(recipient);
        request.setPatUser(patUser);
        return this.patOnePat(request);
    }

    @Override
    public String revokeMsg(WxPpWcfRevokeMsgReq request) {
        // 公共校验
        checkClientStatus();
        log.info("[撤回消息]-[消息撤回]-入参打印：{}", request);
        Integer msgId = Integer.valueOf(request.getMsgId());
        // FUNC_REVOKE_MSG_VALUE
        int state = wechatSocketClient.revokeMsg(msgId);
        // 回调处理
        String stringJson = JSON.toJSONString(request);
        sendMsgCallback(stringJson, state);
        return "";
    }

    @Override
    public String revokeMsg(String msgId) {
        WxPpWcfRevokeMsgReq request = new WxPpWcfRevokeMsgReq();
        request.setMsgId(msgId);
        return this.revokeMsg(request);
    }

    @Override
    public String passFriendApply(WxPpWcfPassFriendApplyReq request) {
        // 公共校验
        checkClientStatus();
        log.info("[好友申请]-[通过好友申请]-入参打印：{}", request);
        // FUNC_ACCEPT_FRIEND_VALUE
        int state = wechatSocketClient.acceptNewFriend(request.getEncryptUsername(), request.getTicket(), Integer.parseInt(request.getScene()));
        return "";
    }

    @Override
    public String passFriendApply(String encryptUsername, String ticket, String scene) {
        WxPpWcfPassFriendApplyReq request = new WxPpWcfPassFriendApplyReq();
        request.setEncryptUsername(encryptUsername);
        request.setTicket(ticket);
        request.setScene(scene);
        return this.passFriendApply(request);
    }

    @Override
    public String addFriendGroupMember(WxPpWcfAddFriendGroupMemberReq request) {
        // 公共校验
        checkClientStatus();
        log.info("[添加好友]-[添加群成员为好友]-入参打印：{}", request);
        if (CollectionUtils.isEmpty(request.getGroupMembers())) {
            log.error("[添加好友]-[添加群成员为好友]-待添加人员为空，本次操作取消");
            return "";
        }
        String groupMembersStr = String.join(",", request.getGroupMembers());
        // FUNC_ADD_ROOM_MEMBERS_VALUE
        int state = wechatSocketClient.addChatroomMembers(request.getGroupNo(), groupMembersStr);
        return "";
    }

    @Override
    public String addFriendGroupMember(String groupNo, List<String> groupMembers) {
        WxPpWcfAddFriendGroupMemberReq request = new WxPpWcfAddFriendGroupMemberReq();
        request.setGroupNo(groupNo);
        request.setGroupMembers(groupMembers);
        return this.addFriendGroupMember(request);
    }

    @Override
    public List<WxPpWcfGroupMemberResp> queryGroupMemberList(WxPpWcfGroupMemberReq request) {
        // 公共校验
        checkClientStatus();
        List<WxPpWcfGroupMemberResp> list = new ArrayList<>();
        String weChatUid = queryLoginWeChatUid();
        // 查询群成员
        List<Wcf.DbRow> wcfList = new ArrayList<>();
        if (!ObjectUtils.isEmpty(request.getGroupNo())) {
            wcfList = wechatSocketClient.querySql(DatabaseNameEnum.MICRO_MSG.getCode(),
                "SELECT RoomData FROM ChatRoom WHERE ChatRoomName = '" + request.getGroupNo() + "';");
        }
        // 查询联系人
        List<Wcf.DbRow> dbList = wechatSocketClient.querySql(DatabaseNameEnum.MICRO_MSG.getCode(), "SELECT UserName, NickName, Type FROM Contact;");
        Map<String, String> dbMap = new HashMap<>();
        if (!CollectionUtils.isEmpty(dbList)) {
            for (Wcf.DbRow dbRow : dbList) {
                List<Wcf.DbField> dbFieldList = dbRow.getFieldsList();
                if (!ObjectUtils.isEmpty(dbFieldList)) {
                    WxPpWcfGroupMemberResp vo = new WxPpWcfGroupMemberResp();
                    for (Wcf.DbField dbField : dbFieldList) {
                        if ("UserName".equals(dbField.getColumn())) {
                            vo = new WxPpWcfGroupMemberResp();
                            String content = (String)wechatSocketClient.convertSqlVal(dbField.getType(), dbField.getContent());
                            vo.setWeChatUid(content);
                        }
                        if ("NickName".equals(dbField.getColumn())) {
                            String content = (String)wechatSocketClient.convertSqlVal(dbField.getType(), dbField.getContent());
                            vo.setGroupNickName(content);
                            dbMap.put(vo.getWeChatUid(), vo.getGroupNickName());
                        }
                    }
                }
            }
        }

        // 查询群名称
        if (!CollectionUtils.isEmpty(wcfList)) {
            for (Wcf.DbRow dbRow : wcfList) {
                List<Wcf.DbField> dbFieldList = dbRow.getFieldsList();
                if (!ObjectUtils.isEmpty(dbFieldList)) {
                    WxPpWcfGroupMemberResp vo = new WxPpWcfGroupMemberResp();
                    for (Wcf.DbField dbField : dbFieldList) {
                        if ("RoomData".equals(dbField.getColumn())) {
                            try {
                                byte[] roomDataBytes = dbField.getContent().toByteArray();
                                Wcf.RoomData roomData = Wcf.RoomData.parseFrom(roomDataBytes);
                                for (Wcf.RoomData.RoomMember member : roomData.getMembersList()) {
                                    vo = new WxPpWcfGroupMemberResp();
                                    vo.setWeChatUid(member.getWxid());
                                    // 是否为自己微信
                                    vo.setWhetherSelf(weChatUid.equals(member.getWxid()));
                                    // 是否为企微
                                    vo.setWhetherWork(member.getWxid().endsWith(WxContactsTypeEnum.WORK.getAffix()));
                                    String nickName = member.getName();
                                    if (ObjectUtils.isEmpty(nickName)) {
                                        // 如果没有设置群昵称则默认为微信名称
                                        nickName = dbMap.get(member.getWxid());
                                    }
                                    vo.setGroupNickName(nickName);
                                    vo.setState(String.valueOf(member.getState()));
                                    list.add(vo);
                                }
                            } catch (InvalidProtocolBufferException e) {
                                log.error("异常:", e);
                            }
                        }
                    }
                }
            }
        }
        return list;
    }

    @Override
    public List<WxPpWcfGroupMemberResp> queryGroupMemberList(String groupNo) {
        WxPpWcfGroupMemberReq request = new WxPpWcfGroupMemberReq();
        request.setGroupNo(groupNo);
        return this.queryGroupMemberList(request);
    }

    @Override
    public String inviteGroupMember(WxPpWcfInviteGroupMemberReq request) {
        // 公共校验
        checkClientStatus();
        log.info("[群成员]-[邀请群成员加入]-入参打印：{}", request);
        if (CollectionUtils.isEmpty(request.getGroupMembers())) {
            log.error("[群成员]-[邀请群成员加入]-待邀请进群的人员为空，本次操作取消");
            return "";
        }
        // FUNC_INV_ROOM_MEMBERS_VALUE
        String groupMembersStr = String.join(",", request.getGroupMembers());
        int state = wechatSocketClient.inviteChatroomMembers(request.getGroupNo(), groupMembersStr);
        return "";
    }

    @Override
    public String inviteGroupMember(String groupNo, List<String> groupMembers) {
        WxPpWcfInviteGroupMemberReq request = new WxPpWcfInviteGroupMemberReq();
        request.setGroupNo(groupNo);
        request.setGroupMembers(groupMembers);
        return this.inviteGroupMember(request);
    }

    @Override
    public String deleteGroupMember(WxPpWcfDeleteGroupMemberReq request) {
        // 公共校验
        checkClientStatus();
        log.info("[群成员]-[删除群成员]-入参打印：{}", request);
        if (CollectionUtils.isEmpty(request.getGroupMembers())) {
            log.error("[群成员]-[删除群成员]-待删除的人员为空，本次操作取消");
            return "";
        }
        // FUNC_DEL_ROOM_MEMBERS_VALUE
        String groupMembersStr = String.join(",", request.getGroupMembers());
        int state = wechatSocketClient.delChatroomMembers(request.getGroupNo(), groupMembersStr);
        return "";
    }

    @Override
    public String deleteGroupMember(String groupNo, List<String> groupMembers) {
        WxPpWcfDeleteGroupMemberReq request = new WxPpWcfDeleteGroupMemberReq();
        request.setGroupNo(groupNo);
        request.setGroupMembers(groupMembers);
        return this.deleteGroupMember(request);
    }

    @Override
    public String queryFriendCircle(Integer id) {
        // 公共校验
        checkClientStatus();
        // FUNC_REFRESH_PYQ_VALUE
        int state = wechatSocketClient.refreshPyq(id);
        return "";
    }

    @Override
    public String receiveTransfer(WxPpWcfReceiveTransferReq request) {
        // 公共校验
        checkClientStatus();
        // FUNC_RECV_TRANSFER_VALUE
        int state = wechatSocketClient.receiveTransfer(request.getWeChatUid(), request.getTransferId(), request.getTransactionId());
        return "";
    }

    @Override
    public String receiveTransfer(String weChatUid, String transferId, String transactionId) {
        WxPpWcfReceiveTransferReq request = new WxPpWcfReceiveTransferReq();
        request.setWeChatUid(weChatUid);
        request.setTransferId(transferId);
        request.setTransactionId(transactionId);
        return this.receiveTransfer(request);
    }

    /**
     * 消息回调
     *
     * @param jsonString json数据
     * @param state cmd调用状态
     *
     * @author chandler
     * @date 2024-10-10 23:10
     */
    private void sendMsgCallback(String jsonString, Integer state) {
        // 根据配置文件决定是否回调
        if (MsgCallbackTypeEnum.CLOSE.getCode().equals(weChatFerryProperties.getSendMsgCallbackFlag())
            || (MsgCallbackTypeEnum.SUCCESS.getCode().equals(weChatFerryProperties.getSendMsgCallbackFlag()) && 0 != state)) {
            // 如果是关闭 或者 配置为成功才回调但发送状态为失败 的情况则取消发送
            return;
        }
        // 开启回调，且回调地址不为空
        if (!CollectionUtils.isEmpty(weChatFerryProperties.getSendMsgCallbackUrls())) {
            for (String receiveMsgFwdUrl : weChatFerryProperties.getSendMsgCallbackUrls()) {
                if (!receiveMsgFwdUrl.startsWith("http")) {
                    continue;
                }
                try {
                    String responseStr = HttpClientUtil.doPostJson(receiveMsgFwdUrl, jsonString);
                    if (judgeSuccess(responseStr)) {
                        log.error("[发送消息]-消息回调至外部接口,获取响应状态失败！-URL：{}", receiveMsgFwdUrl);
                    }
                    log.debug("[发送消息]-[回调接收到的消息]-回调消息至：{}", receiveMsgFwdUrl);
                } catch (Exception e) {
                    log.error("[发送消息]-消息回调接口[{}]服务异常：", receiveMsgFwdUrl, e);
                }
            }
        }
    }

    /**
     * 判断调用第三方服务客户端成功状态码
     *
     * @param responseStr 响应参数
     * @return 状态
     *
     * @author chandler
     * @date 2024-10-10 00:10
     */
    private Boolean judgeSuccess(String responseStr) {
        // 默认为通过
        boolean passFlag = false;
        if (!ObjectUtils.isEmpty(responseStr)) {
            JSONObject jSONObject = JSONObject.parseObject(responseStr);
            if (!ObjectUtils.isEmpty(jSONObject) && !CollectionUtils.isEmpty(weChatFerryProperties.getThirdPartyOkCodes())) {
                Map<String, String> codeMap = weChatFerryProperties.getThirdPartyOkCodes();
                for (Map.Entry<String, String> entry : codeMap.entrySet()) {
                    if (!ObjectUtils.isEmpty(jSONObject.get(entry.getKey())) && jSONObject.get(entry.getKey()).equals(entry.getValue())) {
                        passFlag = true;
                        break;
                    }
                }
            }
        }
        return passFlag;
    }

    /**
     * 判断WCF的CMD调用状态
     * 有值 为成功，其他失败
     *
     * @param rsp 响应参数
     * @return 状态
     *
     * @author chandler
     * @date 2024-12-23 21:53
     */
    private int judgeWcfCmdState(Wcf.Response rsp) {
        // 有值 为成功，其他失败
        int state = -1;
        if (rsp != null) {
            state = rsp.getStatus();
        }
        return state;
    }

    /**
     * 转换联系人类型配置
     *
     * @param list 配置参数
     * @return map key:code val:name
     *
     * @author chandler
     * @date 2024-12-24 16:55
     */
    private Map<String, String> convertContactsTypeProperties(List<String> list) {
        Map<String, String> map = new HashMap<>();
        if (!CollectionUtils.isEmpty(list)) {
            for (String str : list) {
                String key = str;
                String val = str;
                // 存在名称则分割
                if (str.contains("|")) {
                    int index = str.indexOf("|");
                    key = str.substring(0, index);
                    val = str.substring(index + 1);
                }
                map.put(key, val);
            }
        }
        return map;
    }

    /**
     * 请求前检测客户端状态
     *
     * @author chandler
     * @date 2025-01-04 18:34
     */
    private void checkClientStatus() {
        if (!wechatSocketClient.isLogin()) {
            throw new BizException("微信客户端未登录或状态异常，请人工关闭本服务之后，退出微信客户端在重启本服务！");
        }
    }

}
