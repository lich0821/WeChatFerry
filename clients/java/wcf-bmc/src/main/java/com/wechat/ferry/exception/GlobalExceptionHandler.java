package com.wechat.ferry.exception;

import java.text.MessageFormat;

import javax.servlet.http.HttpServletRequest;

import org.springframework.core.annotation.Order;
import org.springframework.web.bind.MethodArgumentNotValidException;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.RestControllerAdvice;

import com.wechat.ferry.entity.TResponse;
import com.wechat.ferry.enums.ResponseCodeEnum;

import lombok.extern.slf4j.Slf4j;

/**
 * 全局统一异常
 *
 * @author Simith
 * @date 2021/11/23 23:20
 */
@Slf4j
@Order(-1)
// 表示当前类为全局异常处理器
@RestControllerAdvice
public class GlobalExceptionHandler {

    /**
     * 通用异常-系统级别未知异常
     *
     * @param e 异常信息
     * @return TResponse
     * @author Simith
     * @date 2021/11/23 23:22
     */
    @ExceptionHandler(Exception.class)
    public TResponse<Object> handleException(Exception e) {
        log.error("全局异常信息 ex={}", e.getMessage(), e);
        // 打印堆栈信息
        e.printStackTrace();
        String message = ResponseCodeEnum.FAILED.getMsg() + "：" + e.getMessage();
        return new TResponse<>(ResponseCodeEnum.FAILED, message);
    }

    /**
     * 参数异常
     * 
     * @author chandler
     * @date 2023/4/3 23:26
     * @param request 请求入参
     * @param e 异常消息
     * @return TResponse 返回体
     */
    @ExceptionHandler(value = {MethodArgumentNotValidException.class})
    public TResponse<Object> handleValidationException(HttpServletRequest request, MethodArgumentNotValidException e) {
        log.error("[请求体参数校验不通过]", e);
        String message = e.getBindingResult().getAllErrors().get(0).getDefaultMessage();
        return new TResponse<>(ResponseCodeEnum.PARAM_ERROR, message);
    }

    /**
     * 自定义错误异常
     *
     * @param e 异常信息
     * @return TResponse
     * @author Simith
     * @date 2021/11/23 23:43
     */
    @ExceptionHandler(BizException.class)
    public TResponse<Object> handleBizException(BizException e) {
        // 打印错误
        e.printStackTrace();
        // 获取错误码
        String message = MessageFormat.format(e.getMessage(), e.getArg());
        return new TResponse<>(ResponseCodeEnum.FAILED, message);
    }

}
