## v39.3.3

### 功能列表

| 接口名            | 地址                | 是否支持 | 备注                |
|----------------|-------------------|------|-------------------|
| 查询登录状态         | /loginStatus      | √    | 已测试               |
| 获取登录微信内部识别号UID | /loginWeChatUid   | √    | 已测试               |
| 获取登录微信信息       | /loginWeChatInfo  | √    | 已测试               |
| 获取消息类型列表       | /list/msgType     | √    | 已测试               |
| 获取联系人列表        | /list/contacts    | √    | 已测试               |
| 获取数据库表名称列表     | /list/dbTableName | √    | 已测试               |
| 获取指定数据库中的表列表   | /list/dbTable     | √    | 已测试               |
| 执行数据库查询SQL     | /exec/dbQuerySql  | √    | 已测试               |
| 查询群成员          | /list/groupMember | √    | 已测试               |
| 发送消息汇总入口       | /send/msgMaster   | x    | 预留                |
| 发送文本消息         | /send/textMsg     | x    | 该版本不支持            |
| 发送富文本消息        | /send/richTextMsg | x    | 缩略图参数需要为空，否则会发送失败 |
| 发送XML消息        | /send/xmlMsg      | ?    | 待测试               |
| 发送图片消息         | /send/imageMsg    | √    | 已测试               |
| 发送表情消息         | /send/emojiMsg    | x    | 该版本不支持            |
| 发送文件消息         | /send/fileMsg     | x    | 该版本不支持            |
| 拍一拍群友          | /patOnePat        | √    | 已测试               |

### 已知BUG

- 1.发送表情微信客户端闪退 - `待修复`
- 2.发送富文本包含thumbnailUrl参数会导致消息发送不出去 - `待修复`

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