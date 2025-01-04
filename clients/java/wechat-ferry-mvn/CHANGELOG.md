## v39.3.3

### 版本列表

下载地址：[v39.3.3](https://github.com/lich0821/WeChatFerry/releases/tag/v39.3.3)

| 名称              | 版本        | 文件名                       |
|-----------------|-----------|---------------------------|
| 微信客户端           | 3.9.11.25 | WeChatSetup-3.9.11.25.exe |
| WeChatFerry-SDK | 39.3.3    | v39.3.3.zip               |

### 功能列表

| 接口名            | 地址                     | 是否支持 | 备注                |
|----------------|------------------------|------|-------------------|
| 查询登录状态         | /loginStatus           | ✔️   | 已测试               |
| 获取登录微信内部识别号UID | /loginWeChatUid        | ✔️   | 已测试               |
| 获取登录微信信息       | /loginWeChatInfo       | ✔️   | 已测试               |
| 获取消息类型列表       | /list/msgType          | ✔️   | 已测试               |
| 获取联系人列表        | /list/contacts         | ✔️   | 已测试               |
| 获取数据库表名称列表     | /list/dbTableName      | ✔️   | 已测试               |
| 获取指定数据库中的表列表   | /list/dbTable          | ✔️   | 已测试               |
| 执行数据库查询SQL     | /exec/dbQuerySql       | ✔️   | 已测试               |
| 发送消息汇总入口       | /send/msgMaster        | ❌    | 预留                |
| 发送文本消息         | /send/textMsg          | ✔️   | 已测试               |
| 发送富文本消息        | /send/richTextMsg      | ❌    | 缩略图参数需要为空，否则会发送失败 |
| 发送XML消息        | /send/xmlMsg           | ❌    | 该版本不支持            |
| 发送图片消息         | /send/imageMsg         | ✔️   | 已测试               |
| 发送表情消息         | /send/emojiMsg         | ❌    | 该版本不支持            |
| 发送文件消息         | /send/fileMsg          | ❌    | 该版本不支持            |
| 拍一拍群友          | /patOnePat             | ✔️   | 已测试               |
| 撤回消息           | /revokeMsg             | ❌    | 该版本不支持            |
| 通过好友申请         | /passFriendApply       | ❌    | 该版本不支持            |
| 添加群成员为微信好友     | /addFriend/groupMember | ❔    | 待测试               |
| 查询群成员          | /groupMember/list      | ✔️   | 已测试               |
| 邀请群成员          | /groupMember/invite    | ❔    | 待测试               |
| 删除群成员          | /groupMember/delete    | ❔    | 待测试               |
| 查询朋友圈          | /friendCircle          | ❔    | 待测试               |
| 接收转账           | /receiveTransfer       | ❌    | 该版本不支持            |

### 已知BUG

- 1.发送表情微信客户端闪退 - `待修复`
- 2.发送富文本包含thumbnailUrl参数会导致消息发送不出去 - `待修复`
- 3.发送文件成功之后客户端崩溃 - `待修复`

### 2025-01-04

#### ⛰️ Features

- 退群监测功能关闭，待完善，目前未开启
- 说明文档更新

#### 🐛 Bug fixes
- 微信端退出之后，调用接口返回客户端状态异常提示

### 2024-12-27

#### ⛰️ Features

- 查询群成员返回类新增字段
- 新增退群监测功能
- 说明文档更新

### 2024-12-25

#### ⛰️ Features

- 新增通过好友申请接口
- 新增添加群成员为微信好友接口
- 新增邀请群成员接口
- 新增删除群成员接口
- 新增刷新朋友圈接口
- 新增撤回消息接口
- 接收转账
- 查询群成员请求地址变更
- 消息回调配置文件参数名称修改
- 封装接收到消息之后的业务操作类

### 2024-12-24

#### ⛰️ Features

- 执行数据库查询SQL请求接口API地址修改
- 业务代码迁移至业务类中，并补充日志信息
- 联系人特殊类型支持配置文件自定义
- 接口入参新增为空校验
- 说明文档更新
- 配置文件有更新

### 2024-12-23

#### ⛰️ Features

- 适配SDK39.3.3版本
- wcf.proto文件部分字段类型修改
- 消息转发适配多种消息类型

<br/>

___

<br/><br/>

## v39.2.4 - 推荐✨

### 版本列表

下载地址：[v39.2.4](https://github.com/lich0821/WeChatFerry/releases/tag/v39.2.4)

| 名称              | 版本        | 文件名                       |
|-----------------|-----------|---------------------------|
| 微信客户端           | 3.9.10.27 | WeChatSetup-3.9.10.27.exe |
| WeChatFerry-SDK | 39.2.4    | v39.2.4.zip               |

### 功能列表

| 接口名            | 地址                     | 是否支持 | 备注     |
|----------------|------------------------|------|--------|
| 查询登录状态         | /loginStatus           | ✔️   | 已测试    |
| 获取登录微信内部识别号UID | /loginWeChatUid        | ✔️   | 已测试    |
| 获取登录微信信息       | /loginWeChatInfo       | ✔️   | 已测试    |
| 获取消息类型列表       | /list/msgType          | ✔️   | 已测试    |
| 获取联系人列表        | /list/contacts         | ✔️   | 已测试    |
| 获取数据库表名称列表     | /list/dbTableName      | ✔️   | 已测试    |
| 获取指定数据库中的表列表   | /list/dbTable          | ✔️   | 已测试    |
| 执行数据库查询SQL     | /exec/dbQuerySql       | ✔️   | 已测试    |
| 发送消息汇总入口       | /send/msgMaster        | ❌    | 预留     |
| 发送文本消息         | /send/textMsg          | ✔️   | 已测试    |
| 发送富文本消息        | /send/richTextMsg      | ✔️   | 已测试    |
| 发送XML消息        | /send/xmlMsg           | ❌    | 该版本不支持 |
| 发送图片消息         | /send/imageMsg         | ✔️   | 已测试    |
| 发送表情消息         | /send/emojiMsg         | ✔️   | 已测试    |
| 发送文件消息         | /send/fileMsg          | ✔️   | 已测试    |
| 拍一拍群友          | /patOnePat             | ✔️   | 已测试    |
| 撤回消息           | /revokeMsg             | ❌    | 该版本不支持 |
| 通过好友申请         | /passFriendApply       | ❌    | 该版本不支持 |
| 添加群成员为微信好友     | /addFriend/groupMember | ❔    | 待测试    |
| 查询群成员          | /groupMember/list      | ✔️   | 已测试    |
| 邀请群成员          | /groupMember/invite    | ❔    | 待测试    |
| 删除群成员          | /groupMember/delete    | ❔    | 待测试    |
| 查询朋友圈          | /friendCircle          | ❔    | 待测试    |
| 接收转账           | /receiveTransfer       | ❌    | 该版本不支持 |

<br/>

___