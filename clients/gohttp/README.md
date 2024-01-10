# 微信 REST API

基于 [WeChatFerry RPC](https://github.com/lich0821/WeChatFerry/tree/master/WeChatFerry) 实现，主要特性如下：

- 使用 Go 语言编写，无运行时依赖
- 基于 HTTP 提供操作接口，无缝对接大多数编程语言
- 支持 HTTP 接口授权，参见 [配置说明](#配置说明)
- 消息中的 Xml 尽可能转为 Object

> 此源码仅提供 HTTP REST API 能力，其他能力可参考 [wechat-rest](https://github.com/opentdp/wechat-rest) 相关说明

## 使用方法

1、下载并安装 [WeChatSetup-3.9.2.23](https://github.com/opentdp/wechat-rest/releases/download/v0.0.1/WeChatSetup-3.9.2.23.exe)，其他版本不支持

2、下载 [WeChatFerry](https://github.com/lich0821/WeChatFerry/releases)，解压后，将2个dll文件复制到当前目录，其他文件可忽略

3、双击 `start.bat` 将自动启动微信和接口服务，扫码登录

> 初始化时出现 **Attempt to access invalid address** 错误信息可以忽略

4、浏览器打开 `http://localhost:7600` 查看支持的接口

## 配置说明

启动 `wrest` 时将自动创建一个默认配置文件，完整配置说明可参考开源仓库中的 [config.yml](./config.yml)

- 应使用 `Ctrl + C` 终止 **wrest**，而非直接关闭 **wrest** 窗口
- 若设置了 `token`，请求时需携带 **header** 信息: `Authorization: Bearer $token`

## 开发说明

### 编译须知

由于微信和WCF均为32位应用，所以`go`也必须以`32`位模式编译，务必设置 `GOARCH` 环境变量为 `386`

### 生成 OpenApi 文档

```shell
go get github.com/swaggo/swag/cmd/swag
go install github.com/swaggo/swag/cmd/swag

swag init --parseDependency -g httpd/server.go -o public/swag -ot json
```
