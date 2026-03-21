package com.wechat.ferry.entity;

import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.wechat.ferry.enums.ResponseCodeEnum;

import io.swagger.annotations.ApiModelProperty;
import lombok.Data;
import lombok.ToString;
import lombok.experimental.Accessors;

/**
 * 返回类封装
 */
@Data
@ToString
@Accessors(chain = true)
public class TResponse<T> {

    private static final DateTimeFormatter FORMATTER = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");

    /**
     * 状态码
     */
    @ApiModelProperty(value = "状态码")
    private String code;

    /**
     * 返回信息
     */
    @ApiModelProperty(value = "返回信息")
    private String msg;

    /**
     * 响应时间
     */
    @ApiModelProperty(value = "响应时间")
    private String time;

    /**
     * 响应数据
     */
    @ApiModelProperty(value = "响应数据")
    @JsonInclude(JsonInclude.Include.NON_EMPTY)
    private T data;

    /**
     * 返回类
     *
     * @author chandler
     * @date 2023/4/5 11:31
     * @param t 返回码类
     * @param data 返回数据
     * @return TResponse对象
     */
    public TResponse(IResponse t, T data) {
        this(t);
        this.data = data;
    }

    /**
     * 返回类
     *
     * @author chandler
     * @date 2023/4/5 11:31
     * @param t 返回码类
     * @param msg 返回信息
     * @return TResponse对象
     */
    public TResponse(IResponse t, String msg) {
        this.code = t.getCode();
        this.msg = msg;
        this.time = LocalDateTime.now().format(FORMATTER);
    }

    /**
     * 返回类
     *
     * @author chandler
     * @date 2023/4/5 11:31
     * @param t 返回码类
     * @param msg 返回信息
     * @return TResponse对象
     */
    public TResponse(IResponse t, T data, String msg) {
        this(t, data);
        // 重写返回信息-替换默认的信息
        this.msg = msg;
    }

    public TResponse(IResponse t) {
        this.code = t.getCode();
        this.msg = t.getMsg();
        this.time = LocalDateTime.now().format(FORMATTER);
    }

    public static <T> TResponse<T> ok(IResponse t) {
        return new TResponse<>(t);
    }

    public static <T> TResponse<T> ok(IResponse t, T data) {
        return new TResponse<>(t, data);
    }

    public static <T> TResponse<T> fail(String msg) {
        return new TResponse<>(ResponseCodeEnum.FAILED, msg);
    }

}
