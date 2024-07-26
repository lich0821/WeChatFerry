# WeChatFerry

一个玩微信的工具。更多介绍见：[WeChatFerry: 一个玩微信的工具](https://mp.weixin.qq.com/s/CGLfSaNDy8MyuyPWGjGJ7w)。

<details><summary><font color="red" size="12">免责声明【必读】</font></summary>

本工具仅供学习和技术研究使用，不得用于任何商业或非法行为，否则后果自负。

本工具的作者不对本工具的安全性、完整性、可靠性、有效性、正确性或适用性做任何明示或暗示的保证，也不对本工具的使用或滥用造成的任何直接或间接的损失、责任、索赔、要求或诉讼承担任何责任。

本工具的作者保留随时修改、更新、删除或终止本工具的权利，无需事先通知或承担任何义务。

本工具的使用者应遵守相关法律法规，尊重微信的版权和隐私，不得侵犯微信或其他第三方的合法权益，不得从事任何违法或不道德的行为。

本工具的使用者在下载、安装、运行或使用本工具时，即表示已阅读并同意本免责声明。如有异议，请立即停止使用本工具，并删除所有相关文件。

</details>

|[📖 Python 文档](https://wechatferry.readthedocs.io/)|[📺 Python 视频教程](https://mp.weixin.qq.com/s/APdjGyZ2hllXxyG_sNCfXQ)|[🙋 FAQ](https://mp.weixin.qq.com/s/I6n_6fuQa60CrROWSgq0TA)|
|:-:|:-:|:-:|

👉 [WeChatRobot🤖](https://github.com/lich0821/WeChatRobot)，一个基于 WeChatFerry 的 Python 机器人框架。

|![碲矿](assets/TEQuant.jpg)|![赞赏](assets/QR.jpeg)|
|:-:|:-:|
|后台回复 `WCF` 加群交流|如果你觉得有用|

<details><summary>点击查看功能清单</summary>

* 查询登录状态
* 获取登录账号信息
* 获取消息类型
* 获取联系人
* 获取可查询数据库
* 获取数据库所有表
* 获取语音消息
* 发送文本消息（可 @）
* 发送图片消息
* 发送文件消息
* 发送卡片消息
* 发送 GIF 消息
* 拍一拍群友
* 转发消息
* 开启接收消息
* 关闭接收消息
* 查询数据库
* 获取朋友圈消息
* 下载图片、视频、文件
* 解密图片
* 添加群成员
* 删除群成员
* 邀请群成员

</details>

<details><summary>点击查看支持的客户端</summary>

* Python
* HTTP
* NodeJS

</details>

## 快速开始
### Python
[![PyPi](https://img.shields.io/pypi/v/wcferry.svg)](https://pypi.python.org/pypi/wcferry) [![Downloads](https://static.pepy.tech/badge/wcferry)](https://pypi.python.org/pypi/wcferry) [![Documentation Status](https://readthedocs.org/projects/wechatferry/badge/?version=latest)](https://wechatferry.readthedocs.io/zh/latest/?badge=latest)

* 安装
```sh
pip install --upgrade wcferry
```

* 参考框架：[🤖WeChatRobot](https://github.com/lich0821/WeChatRobot)

### HTTP
#### [wcfrust](https://github.com/lich0821/wcf-client-rust)（基于 Rust）
#### [go_wcf_http](clients/go_wcf_http/README.MD)（基于 Go）

### [NodeJS](clients/wcferry-node)

## 一起开发

> 🚫 非开发用户不需要往下看。
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
环境变量增加 `vcpkg` 所在路径（本项目为：`C:\Tools\vcpkg`）。

#### 安装相关组件

```sh
vcpkg install protobuf[zlib]:x64-windows-static
vcpkg install spdlog:x64-windows-static
vcpkg install nng:x64-windows-static
vcpkg install magic-enum:x64-windows-static
vcpkg install minhook:x64-windows-static
vcpkg integrate install
```

安装完毕后，需要配置 protoc 的环境变量，并确保在命令行下可用，protoc 的路径在 `<vcpkg_install_path>\installed\x86-windows-static\tools\protobuf`

#### 安装 VS2019

#### 安装 Python3

通过微软商店或者 python.org 自行下载均可，注意配置好环境变量，确保 `python3` 在命令行下可用。

安装依赖：
```sh
pip install grpcio-tools==1.48.2
```

</details>

### 编译

使用 VS2019 打开工程，编译即可。编译成功后，在 `WeChatFerry\WeChatFerry\Out` 目录中会看到相应的 DLL 文件。

**注**：如果遇到执行 `protoc` 时的 9009 错误，检查是否是 python3 环境有问题，或者 protoc 命令的环境变量配置不正确。

### 运行
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
    DbgMsg("ListenMessage"); // 封装的 OutputDebugString
    OutputDebugString(L"ListenMessage\n");
    MessageBox(NULL, L"ListenMessage", L"ListenMessage", 0);
```

## 项目结构

```sh
WeChatFerry
├── LICENSE                 # LICENSE
├── README.MD               # 说明
├── WeChatFerry
│   ├── WeChatFerry.sln     # VS2019 工程文件
│   ├── com                 # 公共模块
│   ├── rpc                 # RPC 模块
│   ├── sdk                 # 注入及启动模块
│   ├── smc                 # Silk-Mp3 转换模块
│   └── spy                 # 核心功能实现模块
├── assets
│   ├── QR.jpeg             # 二维码，测试用图
│   ├── TEQuant.jpg         # 二维码，测试用图
│   └── demo.gif            # 示例动图
├── clients
│   ├── go                  # Go 客户端
│   ├── gohttp              # HTTP 客户端
│   ├── http                # HTTP 客户端
│   ├── java                # Java 客户端
│   ├── node                # Node.js 客户端
│   ├── pyauto              # 群友封装的客户端
│   ├── python              # Python 客户端
│   └── rust                # Rust 客户端
└── docs                    # 文档

```

## 版本更新

### v39.2.4

* 修复 wxid 问题

<details><summary>点击查看更多</summary>

客户端越来越多了，版本号开始混乱，所以重新定义了版本号：`w.x.y.z`。

其中：
* `w` 是微信的大版本号，如 `37` (3.7.a.a), `38` (3.8.a.a), `39` (3.9.a.a)
* `x` 是适配的微信的小版本号，从 0 开始
* `y` 是 `WeChatFerry` 的版本，从 0 开始
* `z` 是各客户端的版本，从 0 开始

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
