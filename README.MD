# WeChatFerry

一个让微信封号的工具，千万不要使用。更多介绍见：[~~WeChatFerry: 一个玩微信的工具~~]()。

<details>
<summary><b>⚠️ 免责声明【必读】⚠️</b></summary>

请阅读完整的免责声明：[点击查看](WeChatFerry/DISCLAIMER.md)

</details>

|[📖 Python 文档](https://wechatferry.readthedocs.io/)|[📺 ~~Python 视频教程~~]()|[🙋 ~~FAQ~~]()|
|:-:|:-:|:-:|

👉 [WeChatRobot🤖](https://github.com/lich0821/WeChatRobot)，一个基于 WeChatFerry 的 Python 机器人示例。

<details><summary>点击查看功能清单</summary>

* 获取登录二维码
* 查询登录状态
* 获取登录账号信息
* 获取消息类型
* 获取联系人
* 获取可查询数据库
* 获取数据库所有表
* 获取数据库密钥
* 获取语音消息
* 发送文本消息（可 @）
* 发送图片消息
* 发送文件消息
* 发送卡片消息
* 发送 XML 消息
* 发送 GIF 消息
* 拍一拍群友
* 转发消息
* 开启接收消息
* 关闭接收消息
* 查询数据库
* 获取朋友圈消息
* 下载图片、视频、文件
* 解密图片
* 图片 OCR 识别
* 通过好友申请
* 领取转账
* 添加群成员
* 删除群成员
* 邀请群成员

</details>

<details><summary>点击查看支持的客户端</summary>

* Python
* HTTP
* NodeJS
* Go
* Java
* Rust

</details>

## 感谢大佬们贡献代码

<a href="https://github.com/lich0821/WeChatFerry/graphs/contributors">![](https://contrib.rocks/image?repo=lich0821/WeChatFerry&columns=8&anon=1)</a>

## 快速开始
### Python
[![PyPi](https://img.shields.io/pypi/v/wcferry.svg)](https://pypi.python.org/pypi/wcferry) [![Downloads](https://static.pepy.tech/badge/wcferry)](https://pypi.python.org/pypi/wcferry) [![Documentation Status](https://readthedocs.org/projects/wechatferry/badge/?version=latest)](https://wechatferry.readthedocs.io/zh/latest/?badge=latest)

* 安装
```sh
pip install --upgrade wcferry
```

* 参考示例：[🤖WeChatRobot](https://github.com/lich0821/WeChatRobot)

### HTTP
* [wcfrust](https://github.com/lich0821/wcf-client-rust)（基于 Rust）
* [go_wcf_http](clients/go_wcf_http/README.MD)（基于 Go）
* [wrest-chat](https://github.com/opentdp/wrest-chat)（基于 Go）
* [wcf-http](https://github.com/yuxiaoli/wcf-http)（基于 Python）

### Java
* [java](clients/java/wcferry/README.MD)

### NodeJS
* [wcferry-node](https://github.com/dr-forget/wcferry-node)
* [node-wcferry](https://github.com/stkevintan/node-wcferry)
* [wechatferry](https://github.com/wechatferry/wechatferry)

### C#
* [WeChatFerry.Net](https://github.com/SilkageNet/WeChatFerry.Net) Install using Nuget
* [WeChatFerry-CSharp](https://github.com/send010/WeChatFerry-CSharp)

### Rust
* [wechat-bot](https://github.com/CliffHan/wechat-bot)

### Docker
* [docker_wechat](https://github.com/Saroth/docker_wechat)
* [wechatbot-provider-windows](https://github.com/danni-cool/wechatbot-provider-windows)

## 一起开发

> 🚫 非开发用户不需要往下看。
>
> **开发用户**：可以根据文档和错误提示，自行解决编译错误的人员。

### 安装开发环境

<details><summary>点击查看</summary>

#### 安装 vcpkg

* 安装，参考[Vcpkg: 总览](https://github.com/microsoft/vcpkg/blob/master/README_zh_CN.md)。

```sh
cd C:\Tools
git clone https://github.com/microsoft/vcpkg
.\vcpkg\bootstrap-vcpkg.bat
```

* 添加全局配置：
环境变量增加 `vcpkg` 所在路径（本文为：`C:\Tools\vcpkg`）：
```sh
setx VCPKG_ROOT "C:/Tools/vcpkg" /M
```

* 与 Visual Studio 集成
```sh
vcpkg integrate install # 失败则说明未正确安装或者未正确配置环境变量
```

#### 安装相关组件

编译时会自动安装。但如果需要使用 `protoc.exe`，则需要配置一下 `protoc.exe` 环境变量：`<vcpkg_package_installed_path>\x86-windows-static\x86-windows-static\tools\protobuf`。

（本文为：`C:\Projs\WeChatFerry\WeChatFerry\vcpkg_installed\x86-windows-static\x86-windows-static\tools\protobuf`）

#### 安装 VS2019

#### 安装 Python3

通过微软商店或者 python.org 自行下载均可（注意 `python` 版本不能太高，否则需要自行编译依赖，建议使用 python 3.10），然后配置好环境变量，确保 `python` 在命令行下可用。

安装依赖：
```sh
pip install grpcio-tools==1.48.2
```

</details>

### 编译

使用 VS2019 打开工程（解决方案只有 `x86` 平台），编译即可。编译成功后，在 `WeChatFerry\WeChatFerry\Out` 目录中会看到相应的 DLL 及命令行工具 `wcf.exe`。

**注**：如果遇到执行 `protoc` 时的 9009 错误，检查是否是 python3 环境有问题，或者 protoc 命令的环境变量配置不正确。

### 运行

有两种驱动方式：

**方式一：`wcf.exe` 命令行（推荐，适合子进程编排 / 64 位客户端）**

```sh
wcf.exe start 10086          # 注入并在命令端口 10086 启动 RPC（消息端口为 10087）
wcf.exe start 10086 debug    # 注入 debug 版 spy
wcf.exe stop                 # 卸载 spy
wcf.exe start 10086 --gui    # 出错时弹模态框（默认写 stderr，返回语义化退出码，便于自动化）
```

因 spy 为 32 位，64 位客户端无法直接 `LoadLibrary` 加载 `sdk.dll`，改由子进程调用 `wcf.exe` 驱动注入。语义化退出码：`0` 成功、`2` 参数错误、`3` spy DLL 不存在、`4` 打开/启动微信失败、`5` 注入失败、`6` 初始化 RPC 失败、`7` `.wcf.lock` 读写失败、`8` 停止时微信未运行、`9` 卸载失败、`10` spy 已注入（重复 start）。

**方式二：32 位进程内直接加载 `sdk.dll`（Python 需为 32 位）**

```py
import ctypes
# 加载 sdk.dll （需要绝对路径）
sdk = ctypes.cdll.LoadLibrary("C:/Projs/WeChatFerry/WeChatFerry/Out/sdk.dll")

# 初始化
sdk.WxInitSDK(False, 10086)

# 退出 SDK
sdk.WxDestroySDK()

# 注意关闭 Python 进程
```

### 调试日志
```c
    util::dbg_msg("ListenMessage"); // 封装的 OutputDebugString
    OutputDebugString(L"ListenMessage\n");
    MessageBox(NULL, L"ListenMessage", L"ListenMessage", 0);
```

## 项目结构

```sh
WeChatFerry
├── LICENSE                 # LICENSE
├── README.MD               # 说明
├── WeChatFerry
│   ├── WeChatFerry.sln     # VS2019 工程文件
│   ├── com                 # 公共模块
│   ├── rpc                 # RPC 模块
│   ├── sdk                 # 注入及启动模块
│   ├── smc                 # Silk-Mp3 转换模块
│   ├── spy                 # 核心功能实现模块
│   └── wcf                 # 命令行工具（wcf.exe）
├── assets
│   ├── QR.jpeg             # 二维码，测试用图
│   ├── TEQuant.jpg         # 二维码，测试用图
│   └── demo.gif            # 示例动图
├── clients
│   ├── go                  # Go 客户端
│   ├── go_wcf_http         # Go HTTP 客户端
│   ├── gohttp              # HTTP 客户端
│   ├── http                # HTTP 客户端
│   ├── java                # Java 客户端
│   ├── node                # Node.js 客户端
│   ├── pyauto              # 群友封装的客户端
│   ├── python              # Python 客户端
│   ├── rust                # Rust 客户端
│   └── wcferry-node        # Node.js 客户端
└── docs                    # 文档
```

## 版本更新

### v39.6.0
* 适配微信 `3.9.12.56`，核心 spy 由 x64 迁回 **x86**（32 位）。
* `wcf.exe` 增强：语义化退出码、`--gui` 开关、重复注入防护、按需等待微信启动。

<details><summary>点击查看更多</summary>

客户端越来越多了，版本号开始混乱，所以重新定义了版本号：`w.x.y.z`。

其中：
* `w` 是微信的大版本号，如 `37` (3.7.a.a), `38` (3.8.a.a), `39` (3.9.a.a)
* `x` 是适配的微信的小版本号，从 0 开始
* `y` 是 `WeChatFerry` 的版本，从 0 开始
* `z` 是各客户端的版本，从 0 开始

### v39.5.2
* 没有新功能

### v39.5.1
* 修复邀请进群偶发失败
* 修复获取 wxid 失败

### v39.5.0

* 适配 `3.9.12.51`。

### v39.4.5

* 修复发送 XML 功能。

### v39.4.4

* 实现发送 XML 功能。

### v39.4.3

* 实现通过好友申请功能。

### v39.4.2

* 修复附件下载类型错误。

### v39.4.1

* 修复乱码问题。

### v39.4.0

* 重构代码，适配 `3.9.12.17`。

### v39.3.5

* 代码优化

### v39.3.4

* 实现获取登录二维码

### v39.3.3

* 修复发送文件 / 图片中文路径问题

### v39.3.2

* 修复接收消息问题

### v39.3.0

* 适配 `3.9.11.25`

### v39.2.4

* 修复 wxid 问题

### v39.2.3

* 实现发送 GIF

### v39.2.2

* 修复开启、停止接收消息失败问题

### v39.2.1

* 实现了好多功能（见功能清单）

### v39.2.0

* 开始适配 `3.9.10.27`
* 实现检查登录状态
* 实现获取登录账号信息（wxid、昵称、手机号、数据目录）
* 实现获取消息类型
* 实现开启接收消息
* 实现停止接收消息
* 实现发送文本消息（可 @）
* 实现发送图片消息

### v39.1.0 (2024.04.19)

* 适配 x64 环境
* 重构项目
* 开始适配 `3.9.10.19`

</details>
