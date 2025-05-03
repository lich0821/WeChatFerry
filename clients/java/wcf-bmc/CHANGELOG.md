## 提问须知

- 1.不管是询问还是报告问题，请尽可能的提供详细信息，以下列出常规必须提供的信息
- 2.问题修复基本会在新版本上迭代，所以提出问题之前请先使用新版本试试是否存在该问题

| 类目                   | 值域               | 备注                     |
|----------------------|------------------|------------------------|
| WeChatFerry-SDK(DLL) | V3.9.X.X         | 每次发布的zip文件包会有版本信息      |
| 配套微信版本               | X.X.X            | 微信安装包的官方版本             |
| 客户端/管理端语言            | JAVA/Python/node | 不同的技术栈会有不同的人维护         |
| 是否测试过其他客户端/管理端       | 是/否              | 由此可以判断是SDK层还是客户端/管理的问题 |
| 具体问题                 | ......           | 描述具体的问题                |
| 复现步骤                 | ......           | 描述出能够复现的步骤             |

<br/>

___

<br/><br/>

## v39.5.1

### 版本列表

下载地址：[v39.5.1](https://github.com/lich0821/WeChatFerry/releases/tag/v39.5.1)

| 名称              | 版本        | 文件名                       |
|-----------------|-----------|---------------------------|
| 微信客户端           | 3.9.12.51 | WeChatSetup-3.9.12.51.exe |
| WeChatFerry-SDK | 39.5.1    | v39.5.1.zip               |
| wcf-bmc         | 39.5.1.1  | wcf-bmc-39.5.1.1.jar      |

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
| 发送富文本消息        | /send/richTextMsg      | ❌    | 已知BUG  |
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

### 提示

v39.5.2版本目前会出现注入失败的情况，暂时推迟更新至该版本，故本次采用v39.5.1进行过渡

### 2025-05-03

- 1.修复不同群定制不同功能号的配置文件参数取值错误问题，修改写法

### 2025-05-01

- 1.更新DLL版本迭代
- 2.更新说明文件
- 3.修改群消息策略功能，支持指定对应的群开启对应的功能
- 4.SDK调试模式新增配置文件参数，默认不开启
- 5.proto文件同步更新
- 6.目录名称变更为wcf-bmc

<br/>

___

<br/><br/>

## v39.4.2

### 版本列表

下载地址：[v39.4.2](https://github.com/lich0821/WeChatFerry/releases/tag/v39.4.2)

| 名称              | 版本        | 文件名                       |
|-----------------|-----------|---------------------------|
| 微信客户端           | 3.9.12.17 | WeChatSetup-3.9.12.17.exe |
| WeChatFerry-SDK | 39.4.2    | v39.4.2.zip               |

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
| 发送富文本消息        | /send/richTextMsg      | ❌    | 已知BUG  |
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

### 2025-03-11

- 1.更新DLL版本迭代

<br/>

___

<br/><br/>

## v39.4.1

### 版本列表

下载地址：[v39.4.1](https://github.com/lich0821/WeChatFerry/releases/tag/v39.4.1)

| 名称              | 版本        | 文件名                       |
|-----------------|-----------|---------------------------|
| 微信客户端           | 3.9.12.17 | WeChatSetup-3.9.12.17.exe |
| WeChatFerry-SDK | 39.4.1    | v39.4.1.zip               |

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
| 发送富文本消息        | /send/richTextMsg      | ❌    | 已知BUG  |
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

### 已知BUG

- 1.发送富文本消息无响应 - `待修复`

### 2025-03-08

- 1.更新DLL版本迭代

<br/>

___

<br/><br/>

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
