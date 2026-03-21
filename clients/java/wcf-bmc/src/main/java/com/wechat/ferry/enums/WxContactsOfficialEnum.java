package com.wechat.ferry.enums;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.stream.Collectors;

import org.springframework.util.ObjectUtils;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * 枚举-微信联系人-公众号
 * 
 * @author chandler
 * @date 2024/12/24 16:36
 */
@Getter
@AllArgsConstructor
public enum WxContactsOfficialEnum {

    /**
     * wxid_2876568766325-应用宝-yingyongbao
     */
    YING_YONG_BAO("wxid_2876568766325", "应用宝"),

    /**
     * wxid_2965349653612-i黑马-iheima
     */
    I_HEI_MA("wxid_2965349653612", "i黑马"),

    /**
     * wxid_4302923029011-丁香医生-DingXiangYiSheng
     */
    DING_XIANG_YI_SHENG("wxid_4302923029011", "丁香医生"),

    /**
     * mphelper-公众平台安全助手
     */
    MP_HELPER("mphelper", "公众平台安全助手"),

    /**
     * weixinguanhaozhushou-微信公众平台-weixingongzhong
     */
    WEI_XIN_GONG_ZHONG("weixinguanhaozhushou", "微信公众平台"),

    /**
     * 未匹配上
     */
    UN_MATCH("", null),

    // 结束
    ;

    private final String code;
    private final String name;

    /**
     * map集合 key：code val：枚举
     */
    public static final Map<String, WxContactsOfficialEnum> codeMap =
        Arrays.stream(values()).collect(Collectors.toMap(WxContactsOfficialEnum::getCode, v -> v));

    /**
     * 根据code获取枚举
     */
    public static WxContactsOfficialEnum getCodeMap(String code) {
        return codeMap.getOrDefault(code, UN_MATCH);
    }

    /**
     * map集合 key：code val：名称
     */
    public static Map<String, String> toCodeNameMap() {
        Map<String, String> map = new HashMap<>();
        for (WxContactsOfficialEnum val : WxContactsOfficialEnum.values()) {
            if (!ObjectUtils.isEmpty(val.getCode())) {
                map.put(val.getCode(), val.getName());
            }
        }
        return map;
    }

}
