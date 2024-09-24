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

swagger地址：http://localhost:9201/swagger-ui/index.html

## 参与开发

### 核心依赖

| 依赖            | 版本     | 说明       |
|---------------|--------|----------|
| Spring Boot   | 2.7.18 | 基础框架     |
| protobuf-java | 3.22.2 | rpc      |
| jna           | 5.6.0  | 态访问系统本地库 |
| nng-java      | 1.4.0  | 本地包      |

### 模块结构

```
wcferry-mvn
├─dll                                       核心dll
│  ├─sdk.dll                                sdk文件
│  └─readme.txt                             本目录说明文件
│ 
├─src                                       源
│  ├─main                                   重启命令
│  │  ├─java(com.iamteer)                   java代码目录
│  │  │  ├─config                           配置
│  │  │  ├─entity                           实体
│  │  │  ├─runner                           运行(程序启动中与启动后的自动化任务都放置于此)
│  │  │  ├─service                          业务接口
│  │  │  │  └─impl                          业务实现类
│  │  │  ├─Client.java                      socket客户端
│  │  │  └─WcferryApplication.java          启动类
│  │  │resources                            资源目录
│  │  │  ├─libs                             本程序内置依赖包
│  │  │  ├─proto                            proto文件
│  │  │  ├─win32-x86-64                     依赖程序
│  │  │  ├─application.yml                  本程序主配置文件
│  │  │  └─logback-spring.xml               日志配置文件
│ 
├─pom.xml                                   POM文件
├─README.MD                                 说明文件
│   

```

### 配置说明

本程序主配置文件为 application.yml

#### 配置参数

本程序内置参数统一前缀：wcferry 所有自定义本服务的参数请都放置在此前缀下，如：

```ymal
wcferry:
  # DLL文件位置
  dll-path: /dll/sdk.dll
```

#### 生成proto文件

本程序已经集成了生成proto文件的maven插件，直接install即可生成proto文件，且会在打包程序中去除 src/main/resources/proto
下面的内容，只保留实体类中的文件

默认install会重新根据.proto文件重新生成实体，如果不想被替换，请删除 src/main/resources/proto 下对应的.proto文件即可

### 提交规范

本模块希望大家使用统一提交格式，便于区分

格式：类型(任务号/缺陷号/没有使用0替代): [模块名称]-[子模块名称]-本次修改的说明

如：

```cmd
feat(0): [java]-[wcferry-mvn]-基础类目录划分迁移及代码格式
```

| 名称   | 版本           |
|------|--------------|
| feat | 新功能          |
| fix  | 缺陷           |
| ...  | 其他等git规范中的均可 |


