package com.wechat.ferry.config;

import org.springframework.context.annotation.Configuration;
import org.springframework.web.servlet.config.annotation.ResourceHandlerRegistry;
import org.springframework.web.servlet.config.annotation.ViewControllerRegistry;
import org.springframework.web.servlet.config.annotation.WebMvcConfigurer;

/**
 * 配置类-配置项目首页
 *
 * @author chandler
 * @date 2024-10-04 10:30
 */
@Configuration
public class WebConfig implements WebMvcConfigurer {

    @Override
    public void addViewControllers(ViewControllerRegistry registry) {
        // 将根路径 "/" 的请求重定向到 "/index.html"
        registry.addViewController("/").setViewName("forward:/index.html");
        WebMvcConfigurer.super.addViewControllers(registry);
    }

    @Override
    public void addResourceHandlers(ResourceHandlerRegistry registry) {
        // 添加资源处理器，用于映射静态资源路径
        registry.addResourceHandler("/**")
            // 添加资路径
            .addResourceLocations("classpath:/templates/");
        WebMvcConfigurer.super.addResourceHandlers(registry);
    }

}
