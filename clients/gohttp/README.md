# 微信 REST API

基于 [WeChatFerry RPC](https://github.com/lich0821/WeChatFerry/tree/master/WeChatFerry) 实现的电脑版微信 REST API，使用 Go 语言编写，无第三方运行时依赖。基于 HTTP 提供操作接口，轻松对接任意编程语言。

## 使用方法

1、下载 [WeChatSetup-3.9.2.23](https://github.com/opentdp/wechat-rest/releases/download/v0.0.1/WeChatSetup-3.9.2.23.exe) 和 [Wechat-rest](https://github.com/opentdp/wechat-rest/releases)

2、在一台 Windows 系统电脑上安装刚刚下载的微信

3、同一台电脑上，解压 `Wechat-rest` ，双击 `wrest.exe` 启动接口服务

4、浏览器打开 `http://localhost:7600` 查看支持的接口

## 功能清单

- 检查登录状态
- 获取登录账号 wxid
- 获取登录账号个人信息
- 获取所有消息类型
- 获取完整通讯录
- 获取好友列表
- 获取所有数据库
- 获取数据库中所有表
- 执行 SQL 查询
- 发送文本消息（可 @）
- 发送图片
- 发送文件
- 发送卡片消息
- 保存图片
- 保存语音
- 图片 OCR
- 接受好友申请
- 接收转账
- 刷新朋友圈
- 添加群成员
- 删除群成员
- 获取群列表
- 获取群成员列表
- 获取群成员昵称
- 邀请群成员
- 拍一拍群友
- 开启消息转发
- 停止消息转发

## 生成 OpenApi 文档

```shell
go get github.com/swaggo/swag/cmd/swag
go install github.com/swaggo/swag/cmd/swag

swag init --parseDependency -g httpd/server.go -o public -ot json
```
