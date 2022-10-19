# WeChatFerry
传送信息往返微信的摆渡车。更多介绍见：[微信机器人 DIY 从 0 到 1](https://mp.weixin.qq.com/s/c4ntj-7NTmbpSfF8df6dIw)。

欢迎加群交流，后台回复 `WeChatFerry `：

![碲矿](TEQuant.jpeg)

## 快速开始
下载[最新发布的版本](https://github.com/lich0821/WeChatFerry/releases/latest)。根据技术栈，选择客户端，目前支持：
* C/C++
* Java
* Python

### C/C++
直接运行 `cpp.exe`，会拉起微信（如果还没启动的话），登录之后，会依次执行各功能，具体见 [main.cpp](cpp/main.cpp)。

### Java
使用惯用IDE，打开工程，编译运行。需要注意，使用32位的JDK，具体见内部 [README.MD](java/README.MD)。

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
vcpkg install grpc:x86-windows-static
vcpkg install protobuf[zlib]:x86-windows-static
vcpkg integrate install
```

#### 安装 VS2019

### 生成编解码文件和接口文件
```sh
cd WeChatFerry/proto/
protoc --cpp_out=. -I=. wcf.proto
protoc --grpc_out=. --plugin=protoc-gen-grpc="C:\Tools\vcpkg\packages\grpc_x64-windows\tools\grpc\grpc_cpp_plugin.exe" -I=. wcf.proto
```

执行 `tree .`，可见生成了四个文件：
```txt
.
├── wcf.grpc.pb.cc
├── wcf.grpc.pb.h
├── wcf.pb.cc
├── wcf.pb.h
└── wcf.proto
```

### 编译
使用 VS2019 打开工程，编译即可。

### 运行
双击 `Release` 里的 `cpp.exe`。如果出现中文乱码，则需要将编码改为 `UTF-8`。

## 项目结构
### cpp
C++ 示例应用，介绍如何使用 WeChatFerry。

### java
Java 客户端。

### proto
RPC 消息及接口定义。

### python
Python 客户端。

### sdk
负责将 `spy` 注入微信进程，并启动 gRPC 服务端。

### spy
间谍模块，注入到微信中，通过 RPC 做消息转发工作。

## 版本更新
### v3.7.0.30.11
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
