package com.wechat.ferry.entity;

/**
 * 返回类接口
 */
public interface IResponse {

    /**
     * 状态码
     */
    String getCode();

    /**
     * 返回信息
     */
    String getMsg();

}
