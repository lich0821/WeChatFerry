package com.wechat.ferry.exception;

import java.util.Arrays;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.wechat.ferry.entity.IResponse;
import com.wechat.ferry.enums.ResponseCodeEnum;

import lombok.Data;
import lombok.EqualsAndHashCode;

/**
 * 业务异常类
 */
@Data
@EqualsAndHashCode(callSuper = true)
public class BizException extends RuntimeException {

    /**
     * 返回接口
     */
    @JsonInclude(JsonInclude.Include.NON_EMPTY)
    private final IResponse response;

    /**
     * 返回参数
     */
    @JsonInclude(JsonInclude.Include.NON_EMPTY)
    private transient Object[] arg;

    /**
     * 业务异常构造器
     *
     * @param msg 异常信息
     * @date 2021/11/24 23:58
     */
    public <T extends IResponse> BizException(String msg) {
        super(msg);
        this.response = ResponseCodeEnum.FAILED;
        this.arg = null;
    }

    /**
     * 业务异常构造器
     *
     * @param msg 异常信息
     * @param args 异常参数
     * @date 2021/11/24 23:58
     */
    public <T extends IResponse> BizException(String msg, Object... args) {
        super(msg);
        this.response = ResponseCodeEnum.FAILED;
        this.arg = args;
    }

    /**
     * 业务异常构造器
     *
     * @param t 异常响应码
     * @param args 异常参数
     * @date 2021/11/24 23:59
     */
    public <T extends IResponse> BizException(T t, Object... args) {
        super(Arrays.toString(args));
        this.response = t;
        this.arg = args;
    }

    public <T extends IResponse> BizException(BizException e) {
        this.response = e.getResponse();
        this.arg = e.getArg();
    }

    public <T extends IResponse> BizException(ResponseCodeEnum t, String msg) {
        super(msg);
        this.response = t;
        this.arg = null;
    }

}
