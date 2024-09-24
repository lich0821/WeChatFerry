package com.iamteer.config;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

import springfox.documentation.builders.ApiInfoBuilder;
import springfox.documentation.builders.PathSelectors;
import springfox.documentation.builders.RequestHandlerSelectors;
import springfox.documentation.oas.annotations.EnableOpenApi;
import springfox.documentation.service.ApiInfo;
import springfox.documentation.spi.DocumentationType;
import springfox.documentation.spring.web.plugins.Docket;

/**
 * 配置类-swagger
 * http://localhost:9201/swagger-ui/index.html
 *
 * @author chandler
 * @date 2024-09-24 22:13
 */
@EnableOpenApi
@Configuration
public class SwaggerConfig {

    @Bean
    public Docket api() {
        return new Docket(DocumentationType.SWAGGER_2).select()
            // 替换为您的Controller所在的包路径
            .apis(RequestHandlerSelectors.basePackage("com.iamteer.controller"))
            // 地址
            .paths(PathSelectors.any()).build().apiInfo(apiInfo());
    }

    private ApiInfo apiInfo() {
        return new ApiInfoBuilder()
            // 文档标题
            .title("Wcferry接口文档")
            // 文档路径
            .description("微信机器人底层框架接口文档")
            // 文档版本
            .version("1.0.0").build();
    }

}
