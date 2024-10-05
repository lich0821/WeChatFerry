package com.wechat.ferry.service.impl;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.function.Function;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.util.CollectionUtils;
import org.springframework.util.ObjectUtils;

import com.google.protobuf.ByteString;
import com.google.protobuf.InvalidProtocolBufferException;
import com.wechat.ferry.entity.proto.Wcf;
import com.wechat.ferry.entity.vo.request.WxPpDatabaseSqlReq;
import com.wechat.ferry.entity.vo.request.WxPpDatabaseTableReq;
import com.wechat.ferry.entity.vo.request.WxPpGroupMemberReq;
import com.wechat.ferry.entity.vo.request.WxPpSendCardMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpSendEmojiMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpSendFileMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpSendImageMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpSendTextMsgReq;
import com.wechat.ferry.entity.vo.response.WxPpContactsResp;
import com.wechat.ferry.entity.vo.response.WxPpDatabaseFieldResp;
import com.wechat.ferry.entity.vo.response.WxPpDatabaseRowResp;
import com.wechat.ferry.entity.vo.response.WxPpGroupMemberResp;
import com.wechat.ferry.entity.vo.response.WxPpMsgTypeResp;
import com.wechat.ferry.entity.vo.response.WxPpSendCardMsgResp;
import com.wechat.ferry.entity.vo.response.WxPpSendEmojiMsgResp;
import com.wechat.ferry.entity.vo.response.WxPpSendFileMsgResp;
import com.wechat.ferry.entity.vo.response.WxPpSendImageMsgResp;
import com.wechat.ferry.entity.vo.response.WxPpSendTextMsgResp;
import com.wechat.ferry.enums.SexEnum;
import com.wechat.ferry.enums.WxContactsTypeEnum;
import com.wechat.ferry.handle.WeChatSocketClient;
import com.wechat.ferry.service.WeChatDllService;

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

    @Override
    public Boolean loginStatus() {
        Boolean status = wechatSocketClient.isLogin();
        log.info("[查询]-[登录状态]-status:{}", status);
        return status;
    }

    @Override
    public String queryLoginWeChatId() {
        String weChatNo = wechatSocketClient.getSelfWxId();
        log.info("[查询]-[登录微信编号]-weChatNo:{}", weChatNo);
        return weChatNo;
    }

    @Override
    public List<WxPpMsgTypeResp> queryMsgTypeList() {
        List<WxPpMsgTypeResp> list = new ArrayList<>();
        Map<Integer, String> msgTypeMap = wechatSocketClient.getMsgTypes();
        if (!CollectionUtils.isEmpty(msgTypeMap)) {
            WxPpMsgTypeResp resp;
            for (Map.Entry<Integer, String> entry : msgTypeMap.entrySet()) {
                resp = new WxPpMsgTypeResp();
                resp.setId(entry.getKey());
                resp.setName(entry.getValue());
                list.add(resp);
            }
        }
        log.info("[查询]-[所消息类型]-共查到:{}条", list.size());
        return list;
    }

    @Override
    public List<WxPpContactsResp> queryContactsList() {
        List<WxPpContactsResp> list = new ArrayList<>();
        List<Wcf.RpcContact> rpcContactList = wechatSocketClient.getContacts();
        if (!CollectionUtils.isEmpty(rpcContactList)) {
            for (Wcf.RpcContact rpcContact : rpcContactList) {
                WxPpContactsResp vo = new WxPpContactsResp();
                vo.setWeChatNo(rpcContact.getWxid());
                vo.setWeChatCode(rpcContact.getCode());
                vo.setRemark(rpcContact.getRemark());
                vo.setNickName(rpcContact.getName());
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
                // 是否为企业微信
                if (!ObjectUtils.isEmpty(rpcContact.getWxid())) {
                    if (rpcContact.getWxid().endsWith(WxContactsTypeEnum.WORK.getAffix())) {
                        vo.setType(WxContactsTypeEnum.WORK.getCode());
                        vo.setTypeLabel(WxContactsTypeEnum.WORK.getName());
                    } else if (rpcContact.getWxid().endsWith(WxContactsTypeEnum.GROUP.getAffix())) {
                        vo.setType(WxContactsTypeEnum.GROUP.getCode());
                        vo.setTypeLabel(WxContactsTypeEnum.GROUP.getName());
                    } else if (rpcContact.getWxid().startsWith(WxContactsTypeEnum.OFFICIAL_ACCOUNT.getAffix())) {
                        vo.setType(WxContactsTypeEnum.OFFICIAL_ACCOUNT.getCode());
                        vo.setTypeLabel(WxContactsTypeEnum.OFFICIAL_ACCOUNT.getName());
                    } else {
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
    public List<WxPpDatabaseRowResp> queryDatabaseSql(WxPpDatabaseSqlReq request) {
        List<WxPpDatabaseRowResp> list = new ArrayList<>();
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
                WxPpDatabaseRowResp rowVo = new WxPpDatabaseRowResp();
                List<WxPpDatabaseFieldResp> fieldVoList = new ArrayList<>();
                for (Wcf.DbField dbField : dbRow.getFieldsList()) {
                    WxPpDatabaseFieldResp fieldVo = new WxPpDatabaseFieldResp();
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
    public List<String> queryDatabaseTable(WxPpDatabaseTableReq request) {
        log.info("[查询]-[查询表]-request:{}", request);
        Map<String, String> wcfMap = wechatSocketClient.getDbTables(request.getDatabaseName());

        log.info("[查询]-[查询表]-查到:{}", wcfMap);
        return Collections.emptyList();
    }

    @Override
    public List<WxPpGroupMemberResp> queryGroupMember(WxPpGroupMemberReq request) {
        List<WxPpGroupMemberResp> list = new ArrayList<>();
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
                    WxPpGroupMemberResp vo = new WxPpGroupMemberResp();
                    for (Wcf.DbField dbField : dbFieldList) {
                        if ("UserName".equals(dbField.getColumn())) {
                            vo = new WxPpGroupMemberResp();
                            String content = (String)converterSqlVal(dbField.getType(), dbField.getContent());
                            vo.setWeChatNo(content);
                        }
                        if ("NickName".equals(dbField.getColumn())) {
                            String content = (String)converterSqlVal(dbField.getType(), dbField.getContent());
                            vo.setNickName(content);
                            dbMap.put(vo.getWeChatNo(), vo.getNickName());
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
                    WxPpGroupMemberResp vo = new WxPpGroupMemberResp();
                    for (Wcf.DbField dbField : dbFieldList) {
                        if ("RoomData".equals(dbField.getColumn())) {
                            try {
                                byte[] roomDataBytes = dbField.getContent().toByteArray();
                                Wcf.RoomData roomData = Wcf.RoomData.parseFrom(roomDataBytes);
                                for (Wcf.RoomData.RoomMember member : roomData.getMembersList()) {
                                    vo = new WxPpGroupMemberResp();
                                    vo.setWeChatNo(member.getWxid());
                                    String nickName = member.getName();
                                    if (ObjectUtils.isEmpty(nickName)) {
                                        // 如果没有设置群昵称则默认为微信名称
                                        nickName = dbMap.get(member.getWxid());
                                    }
                                    vo.setNickName(nickName);
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
    public WxPpSendTextMsgResp sendTextMsg(WxPpSendTextMsgReq request) {
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
        int state = wechatSocketClient.sendText(request.getMsgText(), request.getRecipient(), atUser);
        log.info("[发送消息]-[文本消息]-处理结束");
        return null;
    }

    @Override
    public WxPpSendImageMsgResp sendImageMsg(WxPpSendImageMsgReq request) {
        WxPpSendImageMsgResp resp = new WxPpSendImageMsgResp();
        int state = wechatSocketClient.sendImage(request.getPath(), request.getRecipient());
        return null;
    }

    @Override
    public WxPpSendFileMsgResp sendFileMsg(WxPpSendFileMsgReq request) {
        int state = wechatSocketClient.sendFile(request.getPath(), request.getRecipient());
        return null;
    }

    @Override
    public WxPpSendCardMsgResp sendCardMsg(WxPpSendCardMsgReq request) {
        int state = wechatSocketClient.sendXml(request.getRecipient(), request.getXml(), request.getPath(), request.getType());
        return null;
    }

    @Override
    public WxPpSendEmojiMsgResp sendEmojiMsg(WxPpSendEmojiMsgReq request) {
        int state = wechatSocketClient.sendEmotion(request.getPath(), request.getRecipient());
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

}
