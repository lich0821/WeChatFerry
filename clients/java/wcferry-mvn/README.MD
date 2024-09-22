# WeChatFerry Java 客户端 maven版

⚠️ **只支持 Windows** ⚠️

## 快速使用

### 环境准备

| 名称    | 版本        |
|-------|-----------|
| JDK   | 1.8+      |
| Maven | 3.8+      |
| 微信    | 3.9.10.27 |

### 下载文件

* 下载 [最新发布的文件](https://github.com/lich0821/WeChatFerry/releases/latest)

### 使用惯用 IDE，打开工程

可以直接以WeChatFerry为根目录打开

或者以WeChatFerry/clients/java/wcferry-mvn为根目录打开

### 添加Maven

找到 WeChatFerry/clients/java/wcferry-mvn/pom.xml 文件，右键添加到Maven中，会自动下载依赖

### 替换对应版本的dll

把刚下载的最新发布文件解压到本项目中的 dll 文件目录下，直接替换原因文件即可

### 修改配置文件

配置文件：src/main/resources/application.yml

根据自己的dll目录位置修改配置文件

```yaml
# 本服务参数
wcferry:
  # DLL文件位置
  dll-path: E:\WeChatFerry\clients\java\wcferry-mvn\dll\sdk.dll
  # socket端口
  socket-port: 10086
```

### 编译运行

找到 src/main/java/com/iamteer/WcferryApplication.java 类

直接启动即可

### 访问

启动后springboot自身的端口为 9201 socket的端口为 10086

## 参与开发

### 核心依赖

| 依赖            | 版本     | 说明       |
|---------------|--------|----------|
| Spring Boot   | 2.7.18 | 基础框架     |
| protobuf-java | 3.22.2 | rpc      |
| jna           | 5.6.0  | 态访问系统本地库 |
| nng-java      | 1.4.0  | 本地包      |



