# smc — Silk ↔ MP3 语音编解码

本目录是**预编译产物**，并非源码。它把独立项目 **SilkMp3Converter** 的静态库和头文件 vendored 进 WeChatFerry，供 spy 在处理微信语音消息时使用。

- 上游源码：<https://github.com/lich0821/SilkMp3Converter>

## 文件

| 文件 | 说明 |
|------|------|
| `codec.h` | 对外 API 头文件 |
| `Codec.lib` | 预编译静态库（`x86`，与 spy 的 `x86-windows-static` 匹配）|

## 用途

微信语音消息以 **SILK** 编码存储。`spy` 的 `get_audio` 需要把它转成通用格式，调用链见 [`misc_manager.cpp`](../spy/misc_manager.cpp)：

```cpp
#include "codec.h"
// 采样率固定 24000 Hz（微信语音）
Silk2Mp3(silk, mp3path, 24000);
```

`Codec.lib` 在 [`Spy.vcxproj`](../spy/Spy.vcxproj) 的 `AdditionalDependencies` 中链接。

## API

```cpp
// SILK -> MP3：文件到文件 / 内存到文件 / 内存到内存
int Silk2Mp3(std::string inpath, std::string outpath, int sr);
int Silk2Mp3(std::vector<uint8_t> &silk, std::string mp3path, int sr);
int Silk2Mp3(std::vector<uint8_t> &silk, std::vector<uint8_t> &mp3, int sr);

// 底层编解码
int       Mp3Encode(std::vector<uint8_t> &pcm, std::string &mp3path, int32_t sr);
int       Mp3Encode(std::vector<uint8_t> &pcm, std::vector<uint8_t> &mp3, int32_t sr);
DecTime_t SilkDecode(std::vector<uint8_t> &silk, std::vector<uint8_t> &pcm, int32_t sr);
```

`sr` 为采样率（微信语音用 `24000`）。`Silk2Mp3` 系列返回 `0` 表示成功。

## 重建 / 升级

`Codec.lib` 不在本仓库构建，需从上游生成后回填：

1. 克隆并按上游说明编译 <https://github.com/lich0821/SilkMp3Converter>；
2. 编译目标须与 spy 一致（**x86**、静态运行库 `x86-windows-static`），否则链接会报运行库/架构不匹配；
3. 将产出的 `Codec.lib` 与 `codec.h` 覆盖回本目录。
