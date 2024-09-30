package com.iamteer.utils;

import java.util.Map;

import org.springframework.beans.MutablePropertyValues;
import org.springframework.beans.factory.DisposableBean;
import org.springframework.beans.factory.support.DefaultListableBeanFactory;
import org.springframework.beans.factory.support.GenericBeanDefinition;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.annotation.Lazy;
import org.springframework.stereotype.Service;

import com.alibaba.fastjson2.JSON;
import com.alibaba.fastjson2.JSONObject;

import lombok.SneakyThrows;
import lombok.extern.slf4j.Slf4j;

/**
 * Spring 工具类
 * 
 * @author chandler
 * @date 2023-03-30 11:05:49
 */
@Slf4j
@Service
@Lazy(false)
public class SpringContextHolderUtil implements ApplicationContextAware, DisposableBean {

    /**
     * 上下文对象实例
     */
    private static ApplicationContext applicationContext = null;

    /**
     * 获取applicationContext-取得存储在静态变量中的ApplicationContext.
     */
    public static ApplicationContext getApplicationContext() {
        checkApplicationContext();
        return applicationContext;
    }

    /**
     * 实现ApplicationContextAware接口, 注入Context到静态变量中.
     */
    @Override
    public void setApplicationContext(ApplicationContext applicationContext) {
        SpringContextHolderUtil.applicationContext = applicationContext;
    }

    /**
     * 通过name获取Bean-从静态变量applicationContext中取得Bean, 自动转型为所赋值对象的类型.
     */
    @SuppressWarnings("unchecked")
    public static <T> T getBean(String name) {
        checkApplicationContext();
        if (applicationContext.containsBean(name)) {
            return (T)applicationContext.getBean(name);
        }
        return null;
    }

    /**
     * 通过class获取Bean-从静态变量applicationContext中取得Bean, 自动转型为所赋值对象的类型.
     */
    public static <T> T getBean(Class<T> requiredType) {
        checkApplicationContext();
        return applicationContext.getBean(requiredType);
    }

    /**
     * 通过name,以及Clazz返回指定的Bean
     */
    public static <T> T getBean(String name, Class<T> requiredType) {
        return getApplicationContext().getBean(name, requiredType);
    }

    /**
     * 从静态变量ApplicationContext中取得Bean, 自动转型为所赋值对象的类型.
     */
    public static <T> T getBeanOfType(Class<T> clazz) {
        checkApplicationContext();
        return (T)applicationContext.getBeansOfType(clazz);
    }

    /**
     * 清除SpringContextHolder中的ApplicationContext为Null.
     */
    public static void clearHolder() {
        if (log.isDebugEnabled()) {
            log.debug("清除SpringContextHolder中的ApplicationContext:" + applicationContext);
        }
        applicationContext = null;
    }

    /**
     * 发布事件
     * 
     * @param event
     */
    public static void publishEvent(ApplicationEvent event) {
        if (applicationContext == null) {
            return;
        }
        applicationContext.publishEvent(event);
    }

    /**
     * 实现DisposableBean接口, 在Context关闭时清理静态变量.
     */
    @Override
    @SneakyThrows
    public void destroy() {
        SpringContextHolderUtil.clearHolder();
    }

    public static synchronized void registerSingletonBean(String beanName, Class clzz, Map<String, Object> original) {
        checkApplicationContext();
        DefaultListableBeanFactory beanFactory =
            (DefaultListableBeanFactory)SpringContextHolderUtil.getApplicationContext().getAutowireCapableBeanFactory();
        if (beanFactory.containsBean(beanName)) {
            removeBean(beanName);
        }
        GenericBeanDefinition definition = new GenericBeanDefinition();
        // 类class
        definition.setBeanClass(clzz);
        // 属性赋值
        definition.setPropertyValues(new MutablePropertyValues(original));
        // 注册到spring上下文
        beanFactory.registerBeanDefinition(beanName, definition);
    }

    public static synchronized void registerSingletonBean(String beanName, Object obj, Map<String, Object> original) {
        checkApplicationContext();
        DefaultListableBeanFactory beanFactory =
            (DefaultListableBeanFactory)SpringContextHolderUtil.getApplicationContext().getAutowireCapableBeanFactory();
        if (beanFactory.containsBean(beanName)) {
            removeBean(beanName);
        }
        GenericBeanDefinition definition = new GenericBeanDefinition();
        // 类class
        definition.setBeanClass(obj.getClass());
        // 属性赋值
        definition.setPropertyValues(new MutablePropertyValues(original));
        // 注册到spring上下文
        beanFactory.registerBeanDefinition(beanName, definition);
    }

    public static synchronized void registerSingletonBean(String beanName, Object obj) {
        registerSingletonBean(beanName, obj, JSONObject.parseObject(JSON.toJSONString(obj), Map.class));
    }

    /**
     * 删除spring中管理的bean
     *
     * @param beanName
     */
    public static void removeBean(String beanName) {
        ApplicationContext ctx = SpringContextHolderUtil.getApplicationContext();
        DefaultListableBeanFactory acf = (DefaultListableBeanFactory)ctx.getAutowireCapableBeanFactory();
        if (acf.containsBean(beanName)) {
            acf.removeBeanDefinition(beanName);
        }
    }

    private static void checkApplicationContext() {
        if (applicationContext == null) {
            throw new IllegalStateException("applicaitonContext未注入,请在applicationContext.xml中定义SpringContextUtil");
        }
    }
}
