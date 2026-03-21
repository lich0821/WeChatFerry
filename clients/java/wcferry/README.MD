# WeChatFerry Java 客户端

⚠️ **只支持 Windows** ⚠️

## 快速开始

* 下载 [最新发布的文件](https://github.com/lich0821/WeChatFerry/releases/latest)，解压到 `WeChatFerry\clients\java\wcferry\dll`目录下。

* 使用惯用 IDE，打开工程，编译，运行。

## 从头开始

【添加 `wcferry` 依赖】是必须要执行的，工程里不带这几个文件；如果开发新功能，一般需要修改 `wcf.proto`，从而需要重新生成 PB 文件；其他的，一般不需要关注。

### 添加 `nng` 依赖

* 新建一个 `libs` 目录
* 参考 [nng-java](https://github.com/voutilad/nng-java)，编译 jar 包（`nng-java-1.4.0-SNAPSHOT.jar`）
* 添加依赖 `nng-java-1.4.0-SNAPSHOT.jar`

```groovy
implementation 'net.java.dev.jna:jna:5.6.0'
implementation fileTree(dir: 'libs', include: ['*.jar'])
```

#### 编译 / 安装 `mbedtls`

* 下载 [Mbed TLS](https://github.com/Mbed-TLS/mbedtls) 代码并解压
* 新建 `build` 目录，生成 VS 工程

```sh
cd build
cmake ..
```

* 以管理员身份运行 VS2019，打开 `mbed TLS.sln`，选择 `Release` `x64`，编译即可
* [参考](https://github.com/Mbed-TLS/mbedtls)

#### 编译 / 安装 `nng`

* 下载 [nng](https://github.com/nanomsg/nng) 代码并解压
* 新建 `build` 目录，生成 VS 工程

```sh
cmake -DBUILD_SHARED_LIBS=ON -DNNG_ENABLE_TLS=ON -DMBEDTLS_ROOT_DIR="C:\Program Files (x86)\mbed TLS" ..
```

* 打开 `nng.sln`，选择 `Release` `x64`，编译即可
* [参考](https://github.com/nanomsg/nng/issues/953)

### 添加 PB 依赖

```groovy
implementation 'com.google.protobuf:protobuf-java:3.22.2'  // 版本需要和 protoc 一致
```

#### 重新生成 PB 文件

参考 [Protocol Buffer Basics: Java](https://protobuf.dev/getting-started/javatutorial/#problem-domain)。
* 下载 `protoc` 工具（注意，需要与 `com.google.protobuf:protobuf-java` 一致）：
这里下载 [protoc-22.2](https://github.com/protocolbuffers/protobuf/releases/download/v22.2/protoc-22.2-win64.zip)，解压，并将所在路径加入环境变量（以便随处可用）。

* 编译 PB 文件

```sh
# 调整项目路径
cd C:/Projs/WeChatFerry/java/wcferry/src/main/java
protoc.exe --java_out=. --proto_path=C:\Projs\WeChatFerry\WeChatFerry\rpc\proto wcf.proto
```

### 添加 `dll` 依赖

将  `spy.dll` 、 `spy_debug.dll`、 `spy.dll` 添加到 `WeChatFerry\clients\java\wcferry\dll`。

### 其他问题

### 乱码

参考 [这篇文章](https://www.quanxiaoha.com/idea/idea-chinese-garbled-code.html)。
