package com.wechat.ferry.enums;

import com.wechat.ferry.entity.IResponse;

/**
 * 枚举-返回类状态码
 */
public enum ResponseCodeEnum implements IResponse {

    /**
     * 成功-200
     */
    SUCCESS("200", "请求成功"),

    /**
     * 参数错误-400
     */
    PARAM_ERROR("400", "参数错误"),

    /**
     * 401-身份验证失败
     */
    NO_AUTH("401", "身份验证失败"),

    /**
     * 403-您无权访问此资源
     */
    UNAUTHORIZED("403", "您无权访问此资源"),

    /**
     * 404-未找到该资源
     */
    NOT_FOUND("404", "未找到该资源"),

    /**
     * 失败-500
     */
    FAILED("500", "请求失败"),

    ;

    private final String code;
    private final String msg;

    ResponseCodeEnum(String code, String msg) {
        this.code = code;
        this.msg = msg;
    }

    @Override
    public String getCode() {
        return code;
    }

    @Override
    public String getMsg() {
        return msg;
    }

    @Override
    public String toString() {
        return this.name() + "{" + code + '|' + msg + "}";
    }

}
