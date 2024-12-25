package com.wechat.ferry.service.impl;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.function.Function;

import javax.annotation.Resource;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.util.CollectionUtils;
import org.springframework.util.ObjectUtils;

import com.alibaba.fastjson2.JSON;
import com.alibaba.fastjson2.JSONObject;
import com.google.protobuf.ByteString;
import com.google.protobuf.InvalidProtocolBufferException;
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
import com.wechat.ferry.enums.MsgFwdTypeEnum;
import com.wechat.ferry.enums.SexEnum;
import com.wechat.ferry.enums.WxContactsMixedEnum;
import com.wechat.ferry.enums.WxContactsOfficialEnum;
import com.wechat.ferry.enums.WxContactsTypeEnum;
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
        long startTime = System.currentTimeMillis();
        Boolean status = wechatSocketClient.isLogin();
        long endTime = System.currentTimeMillis();
        log.info("[查询]-[登录状态]-耗时：{}ms，status:{}", (endTime - startTime), status);
        return status;
    }

    @Override
    public String queryLoginWeChatUid() {
        long startTime = System.currentTimeMillis();
        String weChatUid = "";
        Wcf.Request req = Wcf.Request.newBuilder().setFuncValue(Wcf.Functions.FUNC_GET_SELF_WXID_VALUE).build();
        Wcf.Response rsp = wechatSocketClient.sendCmd(req);
        if (!ObjectUtils.isEmpty(rsp)) {
            weChatUid = rsp.getStr();
        }
        long endTime = System.currentTimeMillis();
        log.info("[查询]-[登录微信UID]-耗时：{}ms， weChatUid:{}", (endTime - startTime), weChatUid);
        return weChatUid;
    }

    @Override
    public WxPpWcfLoginInfoResp queryLoginWeChatInfo() {
        long startTime = System.currentTimeMillis();
        WxPpWcfLoginInfoResp resp = new WxPpWcfLoginInfoResp();
        Wcf.Request req = Wcf.Request.newBuilder().setFuncValue(Wcf.Functions.FUNC_GET_USER_INFO_VALUE).build();
        Wcf.Response rsp = wechatSocketClient.sendCmd(req);
        if (!ObjectUtils.isEmpty(rsp) && !ObjectUtils.isEmpty(rsp.getUi())) {
            Wcf.UserInfo userInfo = rsp.getUi();
            resp.setWeChatUid(userInfo.getWxid());
            resp.setWeChatNickname(userInfo.getName());
            resp.setPhone(userInfo.getMobile());
            resp.setHomePath(userInfo.getHome());
        }
        long endTime = System.currentTimeMillis();
        log.info("[查询]-[获取登录微信信息]-耗时：{}ms，登录信息:{}", (endTime - startTime), resp);
        return resp;
    }

    @Override
    public List<WxPpWcfMsgTypeResp> queryMsgTypeList() {
        long startTime = System.currentTimeMillis();
        List<WxPpWcfMsgTypeResp> list = new ArrayList<>();
        Wcf.Request req = Wcf.Request.newBuilder().setFuncValue(Wcf.Functions.FUNC_GET_MSG_TYPES_VALUE).build();
        Wcf.Response rsp = wechatSocketClient.sendCmd(req);
        if (!ObjectUtils.isEmpty(rsp)) {
            Map<Integer, String> msgTypeMap = rsp.getTypes().getTypesMap();
            WxPpWcfMsgTypeResp resp;
            for (Map.Entry<Integer, String> entry : msgTypeMap.entrySet()) {
                resp = new WxPpWcfMsgTypeResp();
                resp.setId(entry.getKey());
                resp.setName(entry.getValue());
                list.add(resp);
            }
        }
        long endTime = System.currentTimeMillis();
        log.info("[查询]-[所有消息类型]-共查到:{}条，耗时：{}ms", list.size(), (endTime - startTime));
        return list;
    }

    @Override
    public List<WxPpWcfContactsResp> queryContactsList() {
        long startTime = System.currentTimeMillis();
        List<WxPpWcfContactsResp> list = new ArrayList<>();
        Wcf.Request req = Wcf.Request.newBuilder().setFuncValue(Wcf.Functions.FUNC_GET_CONTACTS_VALUE).build();
        Wcf.Response rsp = wechatSocketClient.sendCmd(req);
        if (!ObjectUtils.isEmpty(rsp) && !ObjectUtils.isEmpty(rsp.getContacts()) && !CollectionUtils.isEmpty(rsp.getContacts().getContactsList())) {
            List<Wcf.RpcContact> rpcContactList = rsp.getContacts().getContactsList();
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
                    // 官方杂号集合
                    Map<String, String> mixedNoMap = WxContactsMixedEnum.toCodeNameMap();
                    mixedNoMap.putAll(convertContactsTypeProperties(weChatFerryProperties.getContactsTypeMixed()));
                    // 公众号
                    Map<String, String> officialMap = WxContactsOfficialEnum.toCodeNameMap();
                    officialMap.putAll(convertContactsTypeProperties(weChatFerryProperties.getContactsTypeOfficial()));

                    // 类型判断,存在优先级的，官方杂号优先级高于微信公众号(如果定义重复了，常规禁止重复，手机端和电脑端分类不同)
                    if (rpcContact.getWxid().endsWith(WxContactsTypeEnum.WORK.getAffix())) {
                        // 企微
                        vo.setType(WxContactsTypeEnum.WORK.getCode());
                        vo.setTypeLabel(WxContactsTypeEnum.WORK.getName());
                    } else if (rpcContact.getWxid().endsWith(WxContactsTypeEnum.GROUP.getAffix()) || rpcContact.getWxid().endsWith("@im.chatroom")) {
                        // 群聊 @im.chatroom 这种是很早之前的格式，单独例举
                        vo.setType(WxContactsTypeEnum.GROUP.getCode());
                        vo.setTypeLabel(WxContactsTypeEnum.GROUP.getName());
                    } else if (mixedNoMap.containsKey(rpcContact.getWxid())) {
                        // 官方杂号
                        vo.setType(WxContactsTypeEnum.OFFICIAL_MIXED_NO.getCode());
                        vo.setTypeLabel(WxContactsTypeEnum.OFFICIAL_MIXED_NO.getName());
                    } else if (rpcContact.getWxid().startsWith(WxContactsTypeEnum.OFFICIAL_ACCOUNT.getAffix())) {
                        // 微信公众号
                        vo.setType(WxContactsTypeEnum.OFFICIAL_ACCOUNT.getCode());
                        vo.setTypeLabel(WxContactsTypeEnum.OFFICIAL_ACCOUNT.getName());
                    } else if (officialMap.containsKey(rpcContact.getWxid())) {
                        vo.setType(WxContactsTypeEnum.OFFICIAL_ACCOUNT.getCode());
                        vo.setTypeLabel(WxContactsTypeEnum.OFFICIAL_ACCOUNT.getName());
                    } else {
                        // 个微
                        vo.setType(WxContactsTypeEnum.PERSON.getCode());
                        vo.setTypeLabel(WxContactsTypeEnum.PERSON.getName());
                    }
                }
                list.add(vo);
            }
        }
        long endTime = System.currentTimeMillis();
        log.info("[查询]-[联系人]-共查到:{}条，耗时：{}ms", list.size(), (endTime - startTime));
        return list;
    }

    @Override
    public List<String> queryDbTableNameList() {
        long startTime = System.currentTimeMillis();
        List<String> list = new ArrayList<>();
        Wcf.Request req = Wcf.Request.newBuilder().setFuncValue(Wcf.Functions.FUNC_GET_DB_NAMES_VALUE).build();
        Wcf.Response rsp = wechatSocketClient.sendCmd(req);
        if (!ObjectUtils.isEmpty(rsp) && !ObjectUtils.isEmpty(rsp.getDbs()) && !CollectionUtils.isEmpty(rsp.getDbs().getNamesList())) {
            list = rsp.getDbs().getNamesList();
        }
        long endTime = System.currentTimeMillis();
        log.info("[查询]-[数据库名称列表]-共查到:{}条，耗时：{}ms", list.size(), (endTime - startTime));
        return list;
    }

    @Override
    public List<WxPpWcfDatabaseTableResp> queryDbTableList(WxPpWcfDatabaseTableReq request) {
        long startTime = System.currentTimeMillis();
        log.info("[查询]-[数据库表列表]-request:{}", request);
        List<WxPpWcfDatabaseTableResp> list = new ArrayList<>();
        Wcf.Request req = Wcf.Request.newBuilder().setFuncValue(Wcf.Functions.FUNC_GET_DB_TABLES_VALUE).setStr(request.getDatabaseName()).build();
        Wcf.Response rsp = wechatSocketClient.sendCmd(req);
        if (!ObjectUtils.isEmpty(rsp) && !ObjectUtils.isEmpty(rsp.getTables()) && !CollectionUtils.isEmpty(rsp.getTables().getTablesList())) {
            WxPpWcfDatabaseTableResp resp;
            for (Wcf.DbTable tbl : rsp.getTables().getTablesList()) {
                resp = new WxPpWcfDatabaseTableResp();
                resp.setTableName(tbl.getName());
                resp.setSql(tbl.getSql());
                list.add(resp);
            }
        }
        long endTime = System.currentTimeMillis();
        log.info("[查询]-[数据库表列表]-共查到:{}条，耗时：{}ms", list.size(), (endTime - startTime));
        return list;
    }

    @Override
    public List<WxPpWcfDatabaseRowResp> execDbQuerySql(WxPpWcfDatabaseSqlReq request) {
        long startTime = System.currentTimeMillis();
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
                        value = convertSqlVal(dbField.getType(), dbField.getContent());
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
        long endTime = System.currentTimeMillis();
        log.info("[查询]-[执行数据库查询SQL]-共查到:{}条，耗时：{}ms", list.size(), (endTime - startTime));
        return list;
    }

    @Override
    public WxPpWcfSendTextMsgResp sendTextMsg(WxPpWcfSendTextMsgReq request) {
        long startTime = System.currentTimeMillis();
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
        Wcf.TextMsg textMsg = Wcf.TextMsg.newBuilder().setMsg(request.getMsgText()).setReceiver(request.getRecipient()).setAters(atUser).build();
        Wcf.Request req = Wcf.Request.newBuilder().setFuncValue(Wcf.Functions.FUNC_SEND_TXT_VALUE).setTxt(textMsg).build();
        log.debug("sendText: {}", wechatSocketClient.bytesToHex(req.toByteArray()));
        Wcf.Response rsp = wechatSocketClient.sendCmd(req);
        // 0 为成功，其他失败
        int state = judgeWcfCmdState(rsp);
        // 转发处理
        String stringJson = JSON.toJSONString(request);
        sendMsgForward(stringJson, state);
        long endTime = System.currentTimeMillis();
        log.info("[发送消息]-[文本消息]-处理结束，耗时：{}ms", (endTime - startTime));
        return null;
    }

    @Override
    public WxPpWcfSendRichTextMsgResp sendRichTextMsg(WxPpWcfSendRichTextMsgReq request) {
        long startTime = System.currentTimeMillis();
        log.info("[发送消息]-[富文本消息]-入参打印：{}", request);
        Wcf.RichText richTextMsg = Wcf.RichText.newBuilder().setName(request.getName()).setAccount(request.getAccount()).setTitle(request.getTitle())
            .setDigest(request.getDigest()).setUrl(request.getJumpUrl()).setThumburl(request.getThumbnailUrl()).setReceiver(request.getRecipient())
            .build();
        Wcf.Request req = Wcf.Request.newBuilder().setFuncValue(Wcf.Functions.FUNC_SEND_RICH_TXT_VALUE).setRt(richTextMsg).build();
        log.debug("sendRichText: {}", wechatSocketClient.bytesToHex(req.toByteArray()));
        Wcf.Response rsp = wechatSocketClient.sendCmd(req);
        int state = judgeWcfCmdState(rsp);
        // 转发处理
        String stringJson = JSON.toJSONString(request);
        sendMsgForward(stringJson, state);
        long endTime = System.currentTimeMillis();
        log.info("[发送消息]-[富文本消息]-处理结束，耗时：{}ms", (endTime - startTime));
        return null;
    }

    @Override
    public WxPpWcfSendXmlMsgResp sendXmlMsg(WxPpWcfSendXmlMsgReq request) {
        long startTime = System.currentTimeMillis();
        log.info("[发送消息]-[XML消息]-入参打印：{}", request);
        int xmlType = 0x21;
        if ("21".equals(request.getXmlType())) {
            // 小程序
            xmlType = 0x21;
        }
        Wcf.XmlMsg xmlMsg = Wcf.XmlMsg.newBuilder().setContent(request.getXmlContent()).setReceiver(request.getRecipient())
            .setPath(request.getResourcePath()).setType(xmlType).build();
        Wcf.Request req = Wcf.Request.newBuilder().setFuncValue(Wcf.Functions.FUNC_SEND_XML_VALUE).setXml(xmlMsg).build();
        log.debug("sendXml: {}", wechatSocketClient.bytesToHex(req.toByteArray()));
        Wcf.Response rsp = wechatSocketClient.sendCmd(req);
        int state = judgeWcfCmdState(rsp);
        // 转发处理
        String stringJson = JSON.toJSONString(request);
        sendMsgForward(stringJson, state);
        long endTime = System.currentTimeMillis();
        log.info("[发送消息]-[XML消息]-处理结束，耗时：{}ms", (endTime - startTime));
        return null;
    }

    @Override
    public WxPpWcfSendImageMsgResp sendImageMsg(WxPpWcfSendImageMsgReq request) {
        long startTime = System.currentTimeMillis();
        log.info("[发送消息]-[图片消息]-入参打印：{}", request);
        WxPpWcfSendImageMsgResp resp = new WxPpWcfSendImageMsgResp();
        Wcf.PathMsg pathMsg = Wcf.PathMsg.newBuilder().setPath(request.getResourcePath()).setReceiver(request.getRecipient()).build();
        Wcf.Request req = Wcf.Request.newBuilder().setFuncValue(Wcf.Functions.FUNC_SEND_IMG_VALUE).setFile(pathMsg).build();
        log.debug("sendImage: {}", wechatSocketClient.bytesToHex(req.toByteArray()));
        Wcf.Response rsp = wechatSocketClient.sendCmd(req);
        int state = judgeWcfCmdState(rsp);
        // 转发处理
        String stringJson = JSON.toJSONString(request);
        sendMsgForward(stringJson, state);
        long endTime = System.currentTimeMillis();
        log.info("[发送消息]-[图片消息]-处理结束，耗时：{}ms", (endTime - startTime));
        return null;
    }

    @Deprecated
    @Override
    public WxPpWcfSendEmojiMsgResp sendEmojiMsg(WxPpWcfSendEmojiMsgReq request) {
        long startTime = System.currentTimeMillis();
        log.info("[发送消息]-[表情消息]-入参打印：{}", request);
        Wcf.PathMsg pathMsg = Wcf.PathMsg.newBuilder().setPath(request.getResourcePath()).setReceiver(request.getRecipient()).build();
        Wcf.Request req = Wcf.Request.newBuilder().setFuncValue(Wcf.Functions.FUNC_SEND_EMOTION_VALUE).setFile(pathMsg).build();
        log.debug("sendEmotion: {}", wechatSocketClient.bytesToHex(req.toByteArray()));
        Wcf.Response rsp = wechatSocketClient.sendCmd(req);
        int state = judgeWcfCmdState(rsp);
        // 转发处理
        String stringJson = JSON.toJSONString(request);
        sendMsgForward(stringJson, state);
        long endTime = System.currentTimeMillis();
        log.info("[发送消息]-[表情消息]-处理结束，耗时：{}ms", (endTime - startTime));
        return null;
    }

    @Override
    public WxPpWcfSendFileMsgResp sendFileMsg(WxPpWcfSendFileMsgReq request) {
        long startTime = System.currentTimeMillis();
        log.info("[发送消息]-[文件消息]-入参打印：{}", request);
        Wcf.PathMsg pathMsg = Wcf.PathMsg.newBuilder().setPath(request.getResourcePath()).setReceiver(request.getRecipient()).build();
        Wcf.Request req = Wcf.Request.newBuilder().setFuncValue(Wcf.Functions.FUNC_SEND_FILE_VALUE).setFile(pathMsg).build();
        log.debug("sendFile: {}", wechatSocketClient.bytesToHex(req.toByteArray()));
        Wcf.Response rsp = wechatSocketClient.sendCmd(req);
        int state = judgeWcfCmdState(rsp);
        // 转发处理
        String stringJson = JSON.toJSONString(request);
        sendMsgForward(stringJson, state);
        long endTime = System.currentTimeMillis();
        log.info("[发送消息]-[文件消息]-处理结束，耗时：{}ms", (endTime - startTime));
        return null;
    }

    @Override
    public WxPpWcfSendPatOnePatMsgResp patOnePat(WxPpWcfPatOnePatMsgReq request) {
        long startTime = System.currentTimeMillis();
        log.info("[发送消息]-[拍一拍消息]-入参打印：{}", request);
        Wcf.PatMsg patMsg = Wcf.PatMsg.newBuilder().setRoomid(request.getRecipient()).setWxid(request.getPatUser()).build();
        Wcf.Request wcfReq = Wcf.Request.newBuilder().setFuncValue(Wcf.Functions.FUNC_SEND_PAT_MSG_VALUE).setPm(patMsg).build();
        Wcf.Response rsp = wechatSocketClient.sendCmd(wcfReq);
        int state = judgeWcfCmdState(rsp);
        // 转发处理
        String stringJson = JSON.toJSONString(request);
        sendMsgForward(stringJson, state);
        long endTime = System.currentTimeMillis();
        log.info("[发送消息]-[拍一拍消息]-处理结束，耗时：{}ms", (endTime - startTime));
        return null;
    }

    @Override
    public String passFriendApply(WxPpWcfPassFriendApplyReq request) {
        long startTime = System.currentTimeMillis();
        log.info("[好友申请]-[通过好友申请]-入参打印：{}", request);
        Wcf.Verification verification = Wcf.Verification.newBuilder().setV3(request.getApplicant()).setV4(request.getReviewer()).build();
        Wcf.Request req = Wcf.Request.newBuilder().setFuncValue(Wcf.Functions.FUNC_ACCEPT_FRIEND_VALUE).setV(verification).build();
        Wcf.Response rsp = wechatSocketClient.sendCmd(req);
        int state = judgeWcfCmdState(rsp);
        long endTime = System.currentTimeMillis();
        log.info("[好友申请]-[通过好友申请]-处理结束，耗时：{}ms", (endTime - startTime));
        return "";
    }

    @Override
    public String addFriendGroupMember(WxPpWcfAddFriendGroupMemberReq request) {
        long startTime = System.currentTimeMillis();
        log.info("[添加好友]-[添加群成员为好友]-入参打印：{}", request);
        if (CollectionUtils.isEmpty(request.getGroupMembers())) {
            log.error("[添加好友]-[添加群成员为好友]-待添加人员为空，本次操作取消");
            return "";
        }
        String groupMembersStr = String.join(",", request.getGroupMembers());
        Wcf.MemberMgmt memberMgmt = Wcf.MemberMgmt.newBuilder().setRoomid(request.getGroupNo()).setWxids(groupMembersStr).build();
        Wcf.Request req = Wcf.Request.newBuilder().setFuncValue(Wcf.Functions.FUNC_ADD_ROOM_MEMBERS_VALUE).setM(memberMgmt).build();
        Wcf.Response rsp = wechatSocketClient.sendCmd(req);
        int state = judgeWcfCmdState(rsp);
        long endTime = System.currentTimeMillis();
        log.info("[添加好友]-[添加群成员为好友]-处理结束，耗时：{}ms", (endTime - startTime));
        return "";
    }

    @Override
    public List<WxPpWcfGroupMemberResp> queryGroupMemberList(WxPpWcfGroupMemberReq request) {
        long startTime = System.currentTimeMillis();
        List<WxPpWcfGroupMemberResp> list = new ArrayList<>();
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
                            String content = (String)convertSqlVal(dbField.getType(), dbField.getContent());
                            vo.setWeChatUid(content);
                        }
                        if ("NickName".equals(dbField.getColumn())) {
                            String content = (String)convertSqlVal(dbField.getType(), dbField.getContent());
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
        long endTime = System.currentTimeMillis();
        log.info("[查询]-[查询群成员]-共查到:{}条，耗时：{}ms", list.size(), (endTime - startTime));
        return list;
    }

    @Override
    public String inviteGroupMember(WxPpWcfInviteGroupMemberReq request) {
        long startTime = System.currentTimeMillis();
        log.info("[群成员]-[邀请群成员加入]-入参打印：{}", request);
        if (CollectionUtils.isEmpty(request.getGroupMembers())) {
            log.error("[群成员]-[邀请群成员加入]-待邀请进群的人员为空，本次操作取消");
            return "";
        }
        String groupMembersStr = String.join(",", request.getGroupMembers());
        Wcf.MemberMgmt memberMgmt = Wcf.MemberMgmt.newBuilder().setRoomid(request.getGroupNo()).setWxids(groupMembersStr).build();
        Wcf.Request req = Wcf.Request.newBuilder().setFuncValue(Wcf.Functions.FUNC_INV_ROOM_MEMBERS_VALUE).setM(memberMgmt).build();
        Wcf.Response rsp = wechatSocketClient.sendCmd(req);
        int state = judgeWcfCmdState(rsp);
        long endTime = System.currentTimeMillis();
        log.info("[群成员]-[邀请群成员加入]-处理结束，耗时：{}ms", (endTime - startTime));
        return "";
    }

    @Override
    public String deleteGroupMember(WxPpWcfDeleteGroupMemberReq request) {
        long startTime = System.currentTimeMillis();
        log.info("[群成员]-[删除群成员]-入参打印：{}", request);
        if (CollectionUtils.isEmpty(request.getGroupMembers())) {
            log.error("[群成员]-[删除群成员]-待删除的人员为空，本次操作取消");
            return "";
        }
        String groupMembersStr = String.join(",", request.getGroupMembers());
        Wcf.MemberMgmt memberMgmt = Wcf.MemberMgmt.newBuilder().setRoomid(request.getGroupNo()).setWxids(groupMembersStr).build();
        Wcf.Request req = Wcf.Request.newBuilder().setFuncValue(Wcf.Functions.FUNC_DEL_ROOM_MEMBERS_VALUE).setM(memberMgmt).build();
        Wcf.Response rsp = wechatSocketClient.sendCmd(req);
        int state = judgeWcfCmdState(rsp);
        long endTime = System.currentTimeMillis();
        log.info("[群成员]-[删除群成员]-处理结束，耗时：{}ms", (endTime - startTime));
        return "";
    }

    /**
     * 获取SQL类型
     *
     * @param type 转换类型
     * @return 函数
     *
     * @author chandler
     * @date 2024-10-05 12:54
     */
    public Function<byte[], Object> getSqlType(int type) {
        Map<Integer, Function<byte[], Object>> sqlTypeMap = new HashMap<>();
        // 初始化SQL_TYPES 根据类型执行不同的Func
        sqlTypeMap.put(1, bytes -> ByteBuffer.wrap(bytes).getInt());
        sqlTypeMap.put(2, bytes -> ByteBuffer.wrap(bytes).getFloat());
        sqlTypeMap.put(3, bytes -> new String(bytes, StandardCharsets.UTF_8));
        sqlTypeMap.put(4, bytes -> bytes);
        sqlTypeMap.put(5, bytes -> null);
        return sqlTypeMap.get(type);
    }

    /**
     * SQL转换类型
     *
     * @param type 转换类型
     * @param content 待转换内容
     *
     * @author chandler
     * @date 2024-10-05 12:54
     */
    public Object convertSqlVal(int type, ByteString content) {
        // 根据每一列的类型转换
        Function<byte[], Object> converter = getSqlType(type);
        if (converter != null) {
            return converter.apply(content.toByteArray());
        } else {
            log.warn("未知的SQL类型: {}", type);
            return content.toByteArray();
        }
    }

    /**
     * 转换艾特用户
     *
     * @param groupNo 群组编号
     * @param atUsers 艾特的用户(名称/微信编号)
     * @return 组装后的艾特用户
     *
     * @author chandler
     * @date 2024-10-03 11:35
     */
    public String dealAtUser(String groupNo, List<String> atUsers) {
        String atUserStr = "";
        if (!CollectionUtils.isEmpty(atUsers)) {
            // 取出要艾特的用户
            for (String atUser : atUsers) {

            }
        }
        return atUserStr;
    }

    /**
     * 消息转发
     *
     * @param jsonString json数据
     * @param state cmd调用状态
     *
     * @author chandler
     * @date 2024-10-10 23:10
     */
    private void sendMsgForward(String jsonString, Integer state) {
        // 根据配置文件决定是否转发
        if (MsgFwdTypeEnum.CLOSE.getCode().equals(weChatFerryProperties.getSendMsgFwdFlag())
            || (MsgFwdTypeEnum.SUCCESS.getCode().equals(weChatFerryProperties.getSendMsgFwdFlag()) && 0 != state)) {
            // 如果是关闭 或者 配置为成功才转发但发送状态为失败 的情况则取消发送
            return;
        }
        // 开启转发，且转发地址不为空
        if (!CollectionUtils.isEmpty(weChatFerryProperties.getSendMsgFwdUrls())) {
            for (String receiveMsgFwdUrl : weChatFerryProperties.getSendMsgFwdUrls()) {
                if (!receiveMsgFwdUrl.startsWith("http")) {
                    continue;
                }
                try {
                    String responseStr = HttpClientUtil.doPostJson(receiveMsgFwdUrl, jsonString);
                    if (judgeSuccess(responseStr)) {
                        log.error("[发送消息]-消息转发外部接口,获取响应状态失败！-URL：{}", receiveMsgFwdUrl);
                    }
                    log.debug("[发送消息]-[转发接收到的消息]-转发消息至：{}", receiveMsgFwdUrl);
                } catch (Exception e) {
                    log.error("[发送消息]-消息转发接口[{}]服务异常：", receiveMsgFwdUrl, e);
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

}
