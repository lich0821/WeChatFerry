# WeChatFerry
一个玩微信的工具。更多介绍见：[WeChatFerry: 一个玩微信的工具](https://mp.weixin.qq.com/s/CGLfSaNDy8MyuyPWGjGJ7w)。

<details><summary><font color="#FF0000" size="5">免责声明【必读】</font></summary>

本工具仅供学习和技术研究使用，不得用于任何商业或非法行为，否则后果自负。

本工具的作者不对本工具的安全性、完整性、可靠性、有效性、正确性或适用性做任何明示或暗示的保证，也不对本工具的使用或滥用造成的任何直接或间接的损失、责任、索赔、要求或诉讼承担任何责任。

本工具的作者保留随时修改、更新、删除或终止本工具的权利，无需事先通知或承担任何义务。

本工具的使用者应遵守相关法律法规，尊重微信的版权和隐私，不得侵犯微信或其他第三方的合法权益，不得从事任何违法或不道德的行为。

本工具的使用者在下载、安装、运行或使用本工具时，即表示已阅读并同意本免责声明。如有异议，请立即停止使用本工具，并删除所有相关文件。

</details>

|[📖 文档](https://wechatferry.readthedocs.io/)|[📺 视频教程](https://mp.weixin.qq.com/s/APdjGyZ2hllXxyG_sNCfXQ)|[🙋 FAQ](https://mp.weixin.qq.com/s/cnERmc0L6YqYQU2lrXCShg)|
|:-:|:-:|:-:|

👉 [WeChatRobot🤖](https://github.com/lich0821/WeChatRobot)，一个基于 WeChatFerry 的 Python 机器人框架。

<details><summary>点击查看功能清单</summary>

* 检查登录状态
* 获取登录账号的 wxid
* 获取消息类型
* 获取所有联系人
* 获取所有好友
* 获取数据库
* 获取某数据库下的表
* 获取用户信息
* 发送文本消息（可 @）
* 发送图片
* 发送文件
* 允许接收消息
* 停止接收消息
* 执行 SQL 查询
* 接受好友申请
* 添加群成员
* 删除群成员
* 解密图片
* 获取朋友圈消息
* 下载图片、文件、视频

</details>

<details><summary>点击查看支持的客户端</summary>

* Go
* HTTP
* Java
* Python
* Rust
</details>

|![碲矿](assets/TEQuant.jpg)|![赞赏](assets/QR.jpeg)|
|:-:|:-:|
|后台回复 `WeChatFerry` 加群交流|如果你觉得有用|

## 快速开始
### Python
[![PyPi](https://img.shields.io/pypi/v/wcferry.svg)](https://pypi.python.org/pypi/wcferry) [![Downloads](https://static.pepy.tech/badge/wcferry)](https://pypi.python.org/pypi/wcferry) [![Documentation Status](https://readthedocs.org/projects/wechatferry/badge/?version=latest)](https://wechatferry.readthedocs.io/zh/latest/?badge=latest)

* 安装
```sh
pip install --upgrade wcferry
```

* 参考框架：[🤖WeChatRobot](https://github.com/lich0821/WeChatRobot)

### HTTP
[![PyPi](https://img.shields.io/pypi/v/wcfhttp.svg)](https://pypi.python.org/pypi/wcfhttp) [![Downloads](https://static.pepy.tech/badge/wcfhttp)](https://pypi.python.org/pypi/wcfhttp) [![Documentation Status](https://readthedocs.org/projects/wechatferry/badge/?version=latest)](https://wechatferry.readthedocs.io/zh/latest/?badge=latest)

* 安装
```sh
pip install --upgrade wcfhttp
```

* 运行
```sh
# 查看版本
wcfhttp -v

# 查看帮助
wcfhttp -h

# 忽略新消息运行
wcfhttp

# 新消息转发到指定地址
wcfhttp --cb http://your_host:your_port/callback
```

---
其余客户端，可下载 [最新发布的版本](https://github.com/lich0821/WeChatFerry/releases/latest)，根据技术栈，选择客户端，将相应 `dll` 和 `exe` 文件放到对应位置。

### Go
参考 [Go README.MD](clients/go/README.md)

### Java
参考 [Java README.MD](clients/java/README.MD)

### Rust
参考 [Rust README.MD](clients/rust/README.MD)

## 一起开发
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
环境变量增加 `vcpkg` 所在路径（本文为：`C:\Tools\vcpkg`）。

#### 安装相关组件
```sh
vcpkg install protobuf[zlib]:x86-windows-static
vcpkg install spdlog:x86-windows-static
vcpkg install nng:x86-windows-static
vcpkg integrate install
```

安装完毕后，需要配置 protoc 的环境变量，并确保在命令行下可用，protoc 的路径在 `<vcpkg_install_path>\installed\x86-windows-static\tools\protobuf`

#### 安装 VS2019

#### 安装 Python3

通过微软商店或者 python.org 自行下载均可，注意配置好环境变量，确保 `python3` 在命令行下可用

</details>

### 编译
使用 VS2019 打开工程，编译即可。

**注**：如果遇到执行 `protoc` 时的 9009 错误，检查是否是 python3 环境有问题，或者 protoc 命令的环境变量配置不正确。

### 运行
有两种方式（二选一）：
1. 运行 `wcf.exe` 启动
```sh
wcf.exe --help
```

2. 双击 `Release` 里的 `launcher.exe`，`启动`。

## 项目结构
```sh
WeChatFerry
├── LICENSE                 # LICENSE
├── README.MD               # 说明
├── WeChatFerry
│   ├── WeChatFerry.sln     # VS2019 工程文件
│   ├── launcher            # 图形版启动器
│   ├── rpc                 # RPC 模块
│   ├── sdk                 # 注入及启动模块
│   ├── spy                 # 核心功能实现模块
│   └── wcf                 # 命令行版启动器
├── assets
│   ├── QR.jpeg             # 二维码，测试用图
│   ├── TEQuant.jpg         # 二维码，测试用图
│   └── demo.gif            # 示例动图
├── clients
│   ├── go                  # Go 客户端
│   ├── http                # HTTP 客户端
│   ├── java                # Java 客户端
│   ├── python              # Python 客户端
│   └── rust                # Rust 客户端
└── docs                    # 文档

```

<details><summary>点击查看</summary>

### go
Go 客户端。

### http
HTTP 客户端。

### java
Java 客户端。

### launcher
> ⚠️ 不要与客户端同时使用！

图形启动器，调用 `sdk` 将 `spy` 注入微信。其他客户端可通过 RPC 连接到 `spy` 进行消息传递。默认监听的地址为 `tcp://0.0.0.0:10086` 和 `tcp://0.0.0.0:10087`。

### python
Python 客户端。

### rpc
RPC 的通信部分和序列化 / 反序列化部分。

### rust
Rust 客户端。

### sdk
负责将 `spy` 注入微信进程，并启动 RPC 服务。

### spy
间谍模块，注入到微信中，通过 RPC 做消息转发工作。

### wcf
跟 `launcher` 类似，命令行版启动器，一般不需要显式运行，各客户端自动调用。
</details>

## 版本更新

### v39.0.6 (2023.11.26)
* 修复下载图片退出问题

<details><summary>点击查看更多</summary>

客户端越来越多了，版本号开始混乱，所以重新定义了版本号：`w.x.y.z`。

其中：
* `w` 是微信的大版本号，如 `37` (3.7.a.a), `38` (3.8.a.a), `39` (3.9.a.a)
* `x` 是适配的微信的小版本号，从 0 开始
* `y` 是 `WeChatFerry` 的版本，从 0 开始
* `z` 是各客户端的版本，从 0 开始

### v39.0.5 (2023.11.22)
* 修复收到某些文件崩溃问题

### v39.0.4 (2023.11.21)
* 下载图片、文件和视频

### v39.0.3 (2023.09.28)
* 修复登录账号昵称超长报错问题

### v39.0.2 (2023.07.16)
* 修复朋友圈消息 `is_group` 为 `True` 问题

### v39.0.1 (2023.07.16)
* 获取朋友圈消息

### v39.0.0 (2023.07.14)
升级到 `3.9.2.23`。

### v37.1.25 (2023.05.07)
更新版本编码。

根据新版本编码规则：
* `WeChatFerry` 的 `v3.7.0.30.25` 应调整为：`v37.1.25`，因为此前曾适配过 `3.7.0.29`。
* Python 客户端 `wcferry` 的 `v3.7.0.30.25` 应该调整为 `v37.1.25.0`
* HTTP 客户端 `wcfhttp` 的 `v3.7.0.30.25` 应该调整为 `v37.1.25.0`

### v3.7.0.30.25 (2023.05.05)
* 修复群消息判断错误
* 修复名片添加好友问题
* 修复获取数据库多余字符问题
* 添加 Python 文档
* Python 客户端发送图片支持网络路径

### v3.7.0.30.24 (2023.04.19)
实现了一个功能。

### v3.7.0.30.23 (2023.04.13)
* 解密图片
* 获取登录账号信息
* 获取联系人备注

### v3.7.0.30.22（2023.04.09）
将监听端口调整为可配置。

### v3.7.0.30.21（2023.03.15）
* 发送表情

### v3.7.0.30.20（2023.03.12）
修复 wxid 获取问题。

### v3.7.0.30.19（2023.03.06）
修复重复消息问题。

### v3.7.0.30.18（2023.03.05）
修复添加好友问题。

### v3.7.0.30.17（2023.03.05）
修复获取登录账号 wxid 问题。

### v3.7.0.30.16（2023.03.04）
将错误码改成错误消息，方便调试。

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

</details>
