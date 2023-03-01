# WeChatFerry
一个玩微信的工具。更多介绍见：[WeChatFerry: 一个玩微信的工具](https://mp.weixin.qq.com/s/CGLfSaNDy8MyuyPWGjGJ7w)。

目前实现的功能：
* 检查登录状态
* 获取登录账号的 wxid
* 获取消息类型
* 获取所有联系人
* 获取所有好友
* 获取数据库
* 获取某数据库下的表
* 发送文本消息（可 @）
* 发送图片
* 发送文件
* 发送 XML
* 允许接收消息
* 停止接收消息
* 执行 SQL 查询
* 接受好友申请
* 添加群成员

![Demo](demo.gif)

👉 [WeChatRobot](https://github.com/lich0821/WeChatRobot)，一个基于WeChatFerry的Python机器人框架。

欢迎加群交流，后台回复 `WeChatFerry `：

![碲矿](TEQuant.jpeg)

## 快速开始
下载[最新发布的版本](https://github.com/lich0821/WeChatFerry/releases/latest)。根据技术栈，选择客户端，目前支持：
* Python

> [gRPC 分支](https://github.com/lich0821/WeChatFerry/tree/grpc) 支持更多客户端：
> * C/C++
> * Java
> * Node.js
> * Python

### Python
* 安装
```sh
pip install wcferry
```

* 运行

参考 [README.MD](python/README.MD)。

## 一起开发
### 安装开发环境
参见 [A gRPC Demo](https://github.com/lich0821/gRpcDemo/blob/wx/README.MD)。
#### 安装 vcpkg
* 安装，参考[Vcpkg: 总览](https://github.com/microsoft/vcpkg/blob/master/README_zh_CN.md)。
```sh
cd C:\Tools
git clone https://github.com/microsoft/vcpkg
.\vcpkg\bootstrap-vcpkg.bat
```

* 添加全局配置：
环境变量增加 `vcpkg` 所在路径（本文为：`C:\Tools\vcpkg`）。

#### 安装 gRPC 相关组件
```sh
vcpkg install protobuf[zlib]:x86-windows-static
vcpkg install spdlog:x86-windows-static
vcpkg install nng:x86-windows-static
vcpkg integrate install
```

#### 安装 VS2019

### 编译
使用 VS2019 打开工程，编译即可。

### 运行
双击 `Release` 里的 `launcher.exe`，`启动`。

## 项目结构
```sh
WeChatFerry
├── LICENSE           # LICENSE
├── README.MD         # 说明
├── TEQuant.jpeg      # 二维码，测试用图
├── WeChatFerry.sln   # VS2019 工程文件
├── demo.gif          # 示例图
├── launcher          # 图形版启动器
├── python            # Python 客户端
├── rpc               # RPC 模块
├── sdk               # 注入及启动模块
├── spy               # 核心功能实现模块
└── wcf               # 命令行版启动器
```

### launcher
图形启动器，调用 `sdk` 将 `spy` 注入微信。其他客户端可通过 RPC 连接到 `spy` 进行消息传递。监听的地址为 `tcp://0.0.0.0:10086` 和 `tcp://0.0.0.0:10087`。

### python
Python 客户端。

### rpc
RPC 的通信部分和序列化 / 反序列化部分。

### sdk
负责将 `spy` 注入微信进程，并启动 RPC 服务。

### spy
间谍模块，注入到微信中，通过 RPC 做消息转发工作。

### wcf
跟 `launcher` 类似，命令行版启动器。

## 版本更新
### v3.7.0.30.15（2023.03.01）
* 发送 xml

### v3.7.0.30.14（2023.02.28）
* 添加群成员

### v3.7.0.30.13（2023.02.27）
去除 gRPC 框架，自定义更轻量的 RPC 轮子 `nnprc`。

### v3.7.0.30.12（2023.01.20）
* 更新 Python 客户端
* 修改监听地址为 `0.0.0.0:10086`
* 增加 `Launcher`，直接注入 `spy`

### v3.7.0.30.11（2022.10.19）
更新 Python 客户端。

### v3.7.0.30-gRPC-2（2022.10.18）
增加 Java 客户端。

### v3.7.0.30-gRPC-1（2022.10.16）
将 RPC 框架切换为 gRPC！

### v3.7.0.30-8（2022.09.25）
* 获取登录账号微信 ID

### v3.7.0.30-7（2022.09.24）
修复群聊有系统消息时会崩溃 bug。后续考虑把消息来源交还给客户端自己区别。

### v3.7.0.30-6（2022.08.21）
* 通过好友验证

### v3.7.0.30-5（2022.08.20）
* 执行 SQL 语句

### v3.7.0.30-4（2022.08.20）
修复群消息 @人 功能。有几点注意事项：
1. `vAtWxids` 是要 `@` 的 `wxid` 清单，以 `,` 分隔。
2. 只有群主才能 `@所有人`，非群主硬发 `@所有人` 会导致消息发不出去；`@所有人` 对应 `vAtWxids` 为 `"notify@all"`。
3. 消息体里 `@` 的数量必须与 `vAtWxids` 里的数量一致，否则消息能发出但 `@` 功能失效。

### v3.7.0.30-3（2022.08.20）
修复可重入 bug。

### v3.7.0.30-2（2022.08.14）
优化 Hook 和 Inject 代码，实现可重入。

### v3.7.0.30-1（2022.08.12）
适配微信 `3.7.0.30`。

### v3.7.0.29-3（2022.08.7）
* 查询数据库，获取库、表。

### v3.7.0.29-2（2022.08.7）
优化 RPC。

### v3.7.0.29-1（2022.08.7）
适配微信 `3.7.0.29`。

### v3.3.0.115-3（2021.08.28）
适配微信 `3.3.0.115`，新增功能：
* 获取所有联系人

### v3.3.0.115-2（2021.08.22）
适配微信 `3.3.0.115`，新增功能：
* 发送图片消息

### v3.3.0.115-1（2021.08.22）
适配微信 `3.3.0.115`。

### v3.0.0.57-1（2021.02.12）
适配微信 `3.0.0.57`，支持功能：
* 登录状态判断
* 接收文本消息
* 发送文本消息
