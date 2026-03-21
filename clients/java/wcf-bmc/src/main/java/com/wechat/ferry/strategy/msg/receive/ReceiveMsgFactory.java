package com.wechat.ferry.strategy.msg.receive;

import java.util.HashMap;
import java.util.Map;

import javax.annotation.Resource;

import org.springframework.beans.factory.InitializingBean;
import org.springframework.context.ApplicationContext;
import org.springframework.stereotype.Component;

import com.wechat.ferry.enums.ReceiveMsgChannelEnum;
import com.wechat.ferry.exception.BizException;

import lombok.extern.slf4j.Slf4j;

/**
 * 策略Context-消息处理-接收消息
 * 可以切换策略的Context（这里实际是Factory）类
 *
 * @author chandler
 * @date 2024-12-25 14:08
 */
@Slf4j
@Component
public class ReceiveMsgFactory implements InitializingBean {

    private static final Map<String, ReceiveMsgStrategy> strategyContainerMap = new HashMap<>();

    /**
     * spring的上下文
     */
    @Resource
    private ApplicationContext applicationContext;

    /**
     * 实现InitializingBean的方法会在启动的时候执行afterPropertiesSet()方法
     * 将Strategy的类都按照定义好的规则（fetchKey），放入Map中
     */
    @Override
    public void afterPropertiesSet() {
        // 初始化时把所有的策略bean放进ioc,用于使用的时候获取
        Map<String, ReceiveMsgStrategy> strategyMap = applicationContext.getBeansOfType(ReceiveMsgStrategy.class);
        strategyMap.forEach((k, v) -> {
            String type = v.getStrategyType();
            log.debug("[策略Context]-[MessageNoticeSendFactory]-策略类型加载：{}", type);
            strategyContainerMap.putIfAbsent(type, v);
        });
    }

    /**
     * 根据策略类型获取不同处理策略类
     *
     * @param strategyType 策略类型
     * @return 策略类
     */
    public static ReceiveMsgStrategy getStrategy(String strategyType) {
        log.debug("[策略Context]-[ReceiveMsgStrategy]-当前策略类型：{}", strategyType);
        // 策略类对应的枚举
        if (!ReceiveMsgChannelEnum.codeMap.containsKey(strategyType)) {
            // 当前的渠道类型未匹配到
            log.error("入参中的策略类型：{}不在枚举(ReceiveMsgChannelEnum)定义范围内，请检查数据合法性！", strategyType);
            // TODO 正常所有的策略都应该在枚举中定义，但考虑到有些人是把项目集成到自己系统中，部分自己的策略类未在枚举中定义，所以这里不抛异常，但是我们建议开启
            // throw new BizException("当前策略未在枚举中定义，请先在枚举中指定");
        }
        ReceiveMsgStrategy handler = strategyContainerMap.get(strategyType);
        if (handler == null) {
            log.error("[策略Context]-[MessageNoticeSendFactory]-策略类型：{}-未找到合适的处理器！", strategyType);
            throw new BizException("未找到合适的处理器！");
        }
        return handler;
    }

    /**
     * 获取全部策略
     * 用于需要全部执行的情况
     *
     * @return 所有的策略类
     */
    public static Map<String, ReceiveMsgStrategy> getAllStrategyContainers() {
        return strategyContainerMap;
    }

}
