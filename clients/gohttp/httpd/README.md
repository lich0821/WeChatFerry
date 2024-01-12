# WeChat Rest Api

基于 [wcferry](https://github.com/opentdp/wechat-rest/tree/master/wcferry) 实现的 HTTP 接口服务，已实现如下功能：

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
- 转发消息给好友
- 转发收到的消息到URL

## 生成 OpenApi 文档

```shell
go get github.com/swaggo/swag/cmd/swag
go install github.com/swaggo/swag/cmd/swag

swag init --parseDependency -g httpd/server.go -o public/swag -ot json
```
