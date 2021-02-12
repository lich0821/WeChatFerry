# WeChatFerry
传送信息往返微信的摆渡车。

## 快速开始
1. 使用 VS2019 编译。
2. 打开 `CMD`，运行 `App.exe`

## 项目结构
### Spy
间谍模块，注入到微信中，通过 RPC 做消息转发工作。

### SDK
RPC 的客户端，封装接口，供其他方调用。

### SDKpy
用于生成 Python 接口。为编译该项目，需要做一些配置：
1. 添加附加包含目录
* Python 头：`C:\Program Files (x86)\Python37-32\Include`
* Pybind11 头：`C:\Projs\.pyenv\pybind11\lib\site-packages\pybind11\include`
*注*：
    1. pybind11 可以通过 Python 安装（本工程安装到虚拟环境里了）：`pip install pybind11`
    2. 然后通过命令查找：`python -m pybind11 --includes`

2. 添加 Python 的库
* `C:\Program Files (x86)\Python37-32\libs`

### App
示例应用，介绍如何调用 SDK。
