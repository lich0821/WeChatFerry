package com.wechat.ferry.aggregation.facade;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.springframework.util.CollectionUtils;
import org.springframework.util.ObjectUtils;

import com.wechat.ferry.config.WeChatFerryProperties;
import com.wechat.ferry.entity.po.wcf.Contact;
import com.wechat.ferry.entity.proto.Wcf;
import com.wechat.ferry.enums.DatabaseNameEnum;
import com.wechat.ferry.enums.WxContactsMixedEnum;
import com.wechat.ferry.enums.WxContactsOfficialEnum;
import com.wechat.ferry.enums.WxContactsTypeEnum;
import com.wechat.ferry.handle.WeChatSocketClient;

import io.swagger.annotations.ApiModelProperty;
import lombok.Data;
import lombok.extern.slf4j.Slf4j;

/**
 * 聚合模型类-联系人
 *
 * @author chandler
 * @date 2023-06-08 22:39:53
 */
@Slf4j
@Data
public class ContactDo extends Contact {

    /**
     * 微信内部识别号UID
     */
    @ApiModelProperty(value = "微信内部识别号UID")
    private String weChatUid;

    /**
     * 联系人类型
     */
    @ApiModelProperty(value = "联系人类型")
    private String contactType;

    /**
     * 展示名称
     * 有备注优先展示备注
     */
    @ApiModelProperty(value = "展示名称")
    private String showName;

    /**
     * 根据自定义SQL查询联系人列表
     *
     * @param wechatSocketClient 通信客户端
     * @param weChatFerryProperties 配置文件
     *
     * @author chandler
     * @date 2024-12-27 16:06
     */
    public List<ContactDo> queryContactListBySql(WeChatSocketClient wechatSocketClient, WeChatFerryProperties weChatFerryProperties) {
        List<ContactDo> list = new ArrayList<>();
        // 查询联系人
        List<Wcf.DbRow> dbContactList = wechatSocketClient.querySql(DatabaseNameEnum.MICRO_MSG.getCode(),
            "SELECT UserName, Alias, DelFlag, Type, VerifyFlag, Remark, NickName, LabelIDList, DomainList, ChatRoomType, PYInitial, QuanPin, RemarkPYInitial, RemarkQuanPin, ChatRoomNotify FROM Contact;");
        if (!CollectionUtils.isEmpty(dbContactList)) {
            for (Wcf.DbRow dbRow : dbContactList) {
                List<Wcf.DbField> dbFieldList = dbRow.getFieldsList();
                if (!ObjectUtils.isEmpty(dbFieldList)) {
                    ContactDo po = new ContactDo();
                    for (Wcf.DbField dbField : dbFieldList) {
                        String content = (String)wechatSocketClient.convertSqlVal(dbField.getType(), dbField.getContent());
                        // 用户名
                        if ("UserName".equals(dbField.getColumn())) {
                            po.setUserName(content);
                            po.setWeChatUid(content);
                            // 设置类型
                            String type = convertContactType(content, weChatFerryProperties);
                            po.setContactType(type);
                        }
                        // 用户名
                        if ("Alias".equals(dbField.getColumn())) {
                            po.setAlias(content);
                        }
                        // 昵称
                        if ("NickName".equals(dbField.getColumn())) {
                            po.setNickname(content);
                        }
                        // 删除标志
                        if ("DelFlag".equals(dbField.getColumn())) {
                            po.setDelFlag(Integer.valueOf(content));
                        }
                        //
                        if ("VerifyFlag".equals(dbField.getColumn())) {
                            po.setVerifyFlag(Integer.valueOf(content));
                        }
                        //
                        if ("Remark".equals(dbField.getColumn())) {
                            po.setRemark(content);
                        }
                        //
                        if ("LabelIDList".equals(dbField.getColumn())) {
                            po.setLabelIdList(content);
                        }
                        //
                        if ("DomainList".equals(dbField.getColumn())) {
                            po.setDomainList(content);
                        }
                        //
                        if ("ChatRoomType".equals(dbField.getColumn())) {
                            po.setChatRoomType(Integer.valueOf(content));
                        }
                    }
                    list.add(po);
                }
            }
        }
        return list;
    }

    /**
     * 转换联系人类型
     * 类型判断,存在优先级的，官方杂号优先级高于微信公众号(如果定义重复了，常规禁止重复，手机端和电脑端分类不同)
     *
     * @param weChatUid 微信识别号
     * @param weChatFerryProperties 配置文件
     *
     * @author chandler
     * @date 2024-12-27 15:56
     */
    public static String convertContactType(String weChatUid, WeChatFerryProperties weChatFerryProperties) {
        String type = "";
        // 官方杂号集合
        Map<String, String> mixedNoMap = WxContactsMixedEnum.toCodeNameMap();
        mixedNoMap.putAll(convertContactsTypeProperties(weChatFerryProperties.getContactsTypeMixed()));
        // 公众号
        Map<String, String> officialMap = WxContactsOfficialEnum.toCodeNameMap();
        officialMap.putAll(convertContactsTypeProperties(weChatFerryProperties.getContactsTypeOfficial()));

        // 类型判断,存在优先级的，官方杂号优先级高于微信公众号(如果定义重复了，常规禁止重复，手机端和电脑端分类不同)
        if (weChatUid.endsWith(WxContactsTypeEnum.WORK.getAffix())) {
            // 企微
            type = WxContactsTypeEnum.WORK.getCode();
        } else if (weChatUid.endsWith(WxContactsTypeEnum.GROUP.getAffix()) || weChatUid.endsWith("@im.chatroom")) {
            // 群聊 @im.chatroom 这种是很早之前的格式，单独例举
            type = WxContactsTypeEnum.GROUP.getCode();
        } else if (mixedNoMap.containsKey(weChatUid)) {
            // 官方杂号
            type = WxContactsTypeEnum.OFFICIAL_MIXED_NO.getCode();
        } else if (weChatUid.startsWith(WxContactsTypeEnum.OFFICIAL_ACCOUNT.getAffix())) {
            // 微信公众号
            type = WxContactsTypeEnum.OFFICIAL_ACCOUNT.getCode();
        } else if (officialMap.containsKey(weChatUid)) {
            type = WxContactsTypeEnum.OFFICIAL_ACCOUNT.getCode();
        } else {
            // 个微
            type = WxContactsTypeEnum.PERSON.getCode();
        }
        return type;
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
    public static Map<String, String> convertContactsTypeProperties(List<String> list) {
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
