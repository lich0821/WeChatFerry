package com.wechat.ferry.service.impl;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Collections;
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
import com.wechat.ferry.entity.vo.request.WxPpWcfDatabaseSqlReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfDatabaseTableReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfGroupMemberReq;
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
import com.wechat.ferry.enums.SexEnum;
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
        Boolean status = wechatSocketClient.isLogin();
        log.info("[查询]-[登录状态]-status:{}", status);
        return status;
    }

    @Override
    public String queryLoginWeChatUid() {
        String weChatUid = wechatSocketClient.getSelfWxId();
        log.info("[查询]-[登录微信UID]-weChatUid:{}", weChatUid);
        return weChatUid;
    }

    @Override
    public WxPpWcfLoginInfoResp queryLoginWeChatInfo() {
        WxPpWcfLoginInfoResp resp = new WxPpWcfLoginInfoResp();
        Wcf.UserInfo userInfo = wechatSocketClient.getUserInfo();
        if (!ObjectUtils.isEmpty(userInfo)) {
            resp.setWeChatUid(userInfo.getWxid());
            resp.setWeChatNickname(userInfo.getName());
            resp.setPhone(userInfo.getMobile());
            resp.setHomePath(userInfo.getHome());
        }
        log.info("[查询]-[获取登录微信信息]-resp:{}", resp);
        return resp;
    }

    @Override
    public List<WxPpWcfMsgTypeResp> queryMsgTypeList() {
        List<WxPpWcfMsgTypeResp> list = new ArrayList<>();
        Map<Integer, String> msgTypeMap = wechatSocketClient.getMsgTypes();
        if (!CollectionUtils.isEmpty(msgTypeMap)) {
            WxPpWcfMsgTypeResp resp;
            for (Map.Entry<Integer, String> entry : msgTypeMap.entrySet()) {
                resp = new WxPpWcfMsgTypeResp();
                resp.setId(entry.getKey());
                resp.setName(entry.getValue());
                list.add(resp);
            }
        }
        log.info("[查询]-[所有消息类型]-共查到:{}条", list.size());
        return list;
    }

    @Override
    public List<WxPpWcfContactsResp> queryContactsList() {
        List<WxPpWcfContactsResp> list = new ArrayList<>();
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
                    List<String> mixedNoList = new ArrayList<>();
                    // 朋友推荐消息
                    mixedNoList.add("fmessage");
                    // 语音记事本
                    mixedNoList.add("medianote");
                    // 漂流瓶
                    mixedNoList.add("floatbottle");
                    // 文件传输助手
                    mixedNoList.add("filehelper");
                    // 新闻
                    mixedNoList.add("newsapp");
                    // 微信团队
                    mixedNoList.add("weixin");
                    // 微信支付 wxzhifu
                    // mixedNoList.add("gh_3dfda90e39d6");
                    // 微信公开课 wx-gongkaike
                    // mixedNoList.add("gh_c46cbbfa1de9");
                    // 微信运动 WeRun-WeChat
                    // mixedNoList.add("gh_43f2581f6fd6");
                    // 微信游戏 game
                    // mixedNoList.add("gh_25d9ac85a4bc");
                    // 微信开发者
                    // mixedNoList.add("gh_56fc3b00cc4f");
                    // 微信搜一搜 wechat_search
                    // mixedNoList.add("gh_f08f54ae25a4");

                    if (rpcContact.getWxid().endsWith(WxContactsTypeEnum.WORK.getAffix())) {
                        // 企微
                        vo.setType(WxContactsTypeEnum.WORK.getCode());
                        vo.setTypeLabel(WxContactsTypeEnum.WORK.getName());
                    } else if (rpcContact.getWxid().endsWith(WxContactsTypeEnum.GROUP.getAffix()) || rpcContact.getWxid().endsWith("@im.chatroom")) {
                        // 群聊 @im.chatroom 这种是很早之前的格式，单独例举
                        vo.setType(WxContactsTypeEnum.GROUP.getCode());
                        vo.setTypeLabel(WxContactsTypeEnum.GROUP.getName());
                    } else if (mixedNoList.contains(rpcContact.getWxid())) {
                        // 官方杂号
                        vo.setType(WxContactsTypeEnum.OFFICIAL_MIXED_NO.getCode());
                        vo.setTypeLabel(WxContactsTypeEnum.OFFICIAL_MIXED_NO.getName());
                    } else if (rpcContact.getWxid().startsWith(WxContactsTypeEnum.OFFICIAL_ACCOUNT.getAffix())) {
                        // 微信公众号
                        vo.setType(WxContactsTypeEnum.OFFICIAL_ACCOUNT.getCode());
                        vo.setTypeLabel(WxContactsTypeEnum.OFFICIAL_ACCOUNT.getName());
                    } else if ("wxid_2876568766325".equals(rpcContact.getWxid()) || "wxid_2965349653612".equals(rpcContact.getWxid())
                        || "wxid_4302923029011".equals(rpcContact.getWxid()) || "mphelper".equals(rpcContact.getWxid())
                        || "weixinguanhaozhushou".equals(rpcContact.getWxid())) {
                        // 应用宝 yingyongbao wxid_2876568766325
                        // i黑马 iheima wxid_2965349653612
                        // 丁香医生 DingXiangYiSheng wxid_4302923029011
                        // 公众平台安全助手 mphelper
                        // 微信公众平台 weixingongzhong weixinguanhaozhushou
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
        log.info("[查询]-[联系人]-共查到:{}条", list.size());
        return list;
    }

    @Override
    public List<WxPpWcfDatabaseRowResp> queryDatabaseSql(WxPpWcfDatabaseSqlReq request) {
        List<WxPpWcfDatabaseRowResp> list = new ArrayList<>();
        List<Wcf.DbRow> wcfList = wechatSocketClient.querySql(request.getDatabaseName(), request.getSqlText());
        if (!CollectionUtils.isEmpty(wcfList)) {
            // List<Map<String, Object>> result = new ArrayList<>();
            // // 获取指定的行
            // for (Wcf.DbRow row : wcfList) {
            // Map<String, Object> rowMap = new HashMap<>();
            // // 遍历每一列
            // for (Wcf.DbField field : row.getFieldsList()) {
            // ByteString content = field.getContent();
            // String column = field.getColumn();
            // int type = field.getType();
            // rowMap.put(column, converterSqlVal(type, content));
            // }
            // result.add(rowMap);
            // }

            for (Wcf.DbRow dbRow : wcfList) {
                WxPpWcfDatabaseRowResp rowVo = new WxPpWcfDatabaseRowResp();
                List<WxPpWcfDatabaseFieldResp> fieldVoList = new ArrayList<>();
                for (Wcf.DbField dbField : dbRow.getFieldsList()) {
                    WxPpWcfDatabaseFieldResp fieldVo = new WxPpWcfDatabaseFieldResp();
                    fieldVo.setType(String.valueOf(dbField.getType()));
                    fieldVo.setColumn(dbField.getColumn());
                    String value = (String)converterSqlVal(dbField.getType(), dbField.getContent());
                    fieldVo.setValue(value);
                    fieldVoList.add(fieldVo);
                }
                rowVo.setFieldList(fieldVoList);
                list.add(rowVo);
            }
        }
        log.info("[查询]-[联系人]-wcfList:{}", wcfList);
        return list;
    }

    @Override
    public List<String> queryDatabaseAllTableName() {
        List<String> list = wechatSocketClient.getDbNames();
        log.info("[查询]-[数据库名称列表]-共查到:{}条", list.size());
        return list;
    }

    @Override
    public List<String> queryDatabaseTable(WxPpWcfDatabaseTableReq request) {
        log.info("[查询]-[查询表]-request:{}", request);
        Map<String, String> wcfMap = wechatSocketClient.getDbTables(request.getDatabaseName());

        log.info("[查询]-[查询表]-查到:{}", wcfMap);
        return Collections.emptyList();
    }

    @Override
    public List<WxPpWcfGroupMemberResp> queryGroupMember(WxPpWcfGroupMemberReq request) {
        List<WxPpWcfGroupMemberResp> list = new ArrayList<>();
        // 查询群成员
        List<Wcf.DbRow> wcfList = new ArrayList<>();
        if (!ObjectUtils.isEmpty(request.getGroupNo())) {
            wcfList =
                wechatSocketClient.querySql("MicroMsg.db", "SELECT RoomData FROM ChatRoom WHERE ChatRoomName = '" + request.getGroupNo() + "';");
        }
        // 查询联系人
        List<Wcf.DbRow> dbList = wechatSocketClient.querySql("MicroMsg.db", "SELECT UserName, NickName, Type FROM Contact;");
        Map<String, String> dbMap = new HashMap<>();
        if (!CollectionUtils.isEmpty(dbList)) {
            for (Wcf.DbRow dbRow : dbList) {
                List<Wcf.DbField> dbFieldList = dbRow.getFieldsList();
                if (!ObjectUtils.isEmpty(dbFieldList)) {
                    WxPpWcfGroupMemberResp vo = new WxPpWcfGroupMemberResp();
                    for (Wcf.DbField dbField : dbFieldList) {
                        if ("UserName".equals(dbField.getColumn())) {
                            vo = new WxPpWcfGroupMemberResp();
                            String content = (String)converterSqlVal(dbField.getType(), dbField.getContent());
                            vo.setWeChatUid(content);
                        }
                        if ("NickName".equals(dbField.getColumn())) {
                            String content = (String)converterSqlVal(dbField.getType(), dbField.getContent());
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
        log.info("[查询]-[查询群成员]-共查到:{}条", list.size());
        return list;
    }

    @Override
    public WxPpWcfSendTextMsgResp sendTextMsg(WxPpWcfSendTextMsgReq request) {
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
        // 0 为成功，其他失败
        int state = wechatSocketClient.sendText(request.getMsgText(), request.getRecipient(), atUser);
        log.info("[发送消息]-[文本消息]-处理结束");
        // 转发处理
        String stringJson = JSON.toJSONString(request);
        if ("2".equals(weChatFerryProperties.getSendMsgFwdFlag())) {
            // 不管消息是否发送成功均转发
            sendMsgForward(stringJson);
        } else if ("3".equals(weChatFerryProperties.getSendMsgFwdFlag()) && 0 == state) {
            // 发送成功才转发
            sendMsgForward(stringJson);
        }
        return null;
    }

    @Override
    public WxPpWcfSendImageMsgResp sendImageMsg(WxPpWcfSendImageMsgReq request) {
        WxPpWcfSendImageMsgResp resp = new WxPpWcfSendImageMsgResp();
        int state = wechatSocketClient.sendImage(request.getResourcePath(), request.getRecipient());
        return null;
    }

    @Override
    public WxPpWcfSendFileMsgResp sendFileMsg(WxPpWcfSendFileMsgReq request) {
        int state = wechatSocketClient.sendFile(request.getResourcePath(), request.getRecipient());
        return null;
    }

    @Override
    public WxPpWcfSendXmlMsgResp sendXmlMsg(WxPpWcfSendXmlMsgReq request) {
        int state = wechatSocketClient.sendXml(request.getRecipient(), request.getXmlContent(), request.getResourcePath(), request.getXmlType());
        return null;
    }

    @Override
    public WxPpWcfSendEmojiMsgResp sendEmojiMsg(WxPpWcfSendEmojiMsgReq request) {
        int state = wechatSocketClient.sendEmotion(request.getResourcePath(), request.getRecipient());
        return null;
    }

    @Override
    public WxPpWcfSendRichTextMsgResp sendRichTextMsg(WxPpWcfSendRichTextMsgReq request) {
        Wcf.RichText richTextMsg = Wcf.RichText.newBuilder().setName(request.getName()).setAccount(request.getAccount()).setTitle(request.getTitle())
            .setDigest(request.getDigest()).setUrl(request.getJumpUrl()).setThumburl(request.getThumbnailUrl()).setReceiver(request.getRecipient())
            .build();
        Wcf.Request wcfReq = Wcf.Request.newBuilder().setFuncValue(Wcf.Functions.FUNC_SEND_RICH_TXT_VALUE).setRt(richTextMsg).build();
        Wcf.Response rsp = wechatSocketClient.sendCmd(wcfReq);
        return null;
    }

    @Override
    public WxPpWcfSendPatOnePatMsgResp patOnePat(WxPpWcfPatOnePatMsgReq request) {
        Wcf.PatMsg patMsg = Wcf.PatMsg.newBuilder().setRoomid(request.getRecipient()).setWxid(request.getPatUser()).build();
        Wcf.Request wcfReq = Wcf.Request.newBuilder().setFuncValue(Wcf.Functions.FUNC_SEND_PAT_MSG_VALUE).setPm(patMsg).build();
        Wcf.Response rsp = wechatSocketClient.sendCmd(wcfReq);
        return null;
    }

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

    public Object converterSqlVal(int type, ByteString content) {
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

    private void sendMsgForward(String jsonString) {
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

}
