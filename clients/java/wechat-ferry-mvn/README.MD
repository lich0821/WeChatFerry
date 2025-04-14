# WeChatFerry Java 客户端 maven版

⚠️ **只支持 Windows** ⚠️

`声明：` 本项目是基于 clients/java/wcferry 项目改造，随着时间推进，项目结构和代码规范逐渐产生分离，使用此项目的人员可参考之前的项目
我们在开发时请尽量保持注释的完整性，便于阅读维护

## 快速使用

### 环境准备

| 名称              | 版本        | 备注 |
|-----------------|-----------|----|
| JDK             | 1.8+      | √  |
| Maven           | 3.8+      | √  |
| 微信客户端           | 3.9.11.25 | √  |
| WeChatFerry-SDK | 39.3.3    | √  |
| MySQL           | 8.0+      | 备用 |

### 下载文件

* 下载 [最新发布的文件](https://github.com/lich0821/WeChatFerry/releases/latest)

### 使用惯用 IDE，打开工程

可以直接以WeChatFerry为根目录打开

或者以WeChatFerry/clients/java/wechat-ferry-mvn为根目录打开

### 添加Maven

找到 WeChatFerry/clients/java/wechat-ferry-mvn/pom.xml 文件，右键添加到Maven中，会自动下载依赖

### 替换对应版本的dll

把刚下载的最新发布文件解压到本项目中的 dll 文件目录下，直接替换原因文件即可

替换 `clients/java/wechat-ferry-mvn/dll` 目录下(也可以在配置文件中改为自定义的目录)

- sdk.dll
- spy.dll
- spy_debug.dll

> 如果之前已经使用本项目启动过微信，此时替换发现替换不了，是因为正则运行的微信客户端正在使用该文件，
> 请退出并关闭微信客户端之后再进行替换

### 修改配置文件

配置文件：src/main/resources/application.yml

根据自己的dll目录位置修改配置文件

```yaml
# 本服务参数
wechat:
  ferry:
    # DLL文件位置
    dll-path: E:\WeChatFerry\clients\java\wechat-ferry-mvn\dll\sdk.dll
    # socket端口
    socket-port: 10086
```

### 数据库修改-目前未使用

为了更好的拓展应用，本客户端后续将采用mysql数据库，请自行安装mysql数据库

### 编译运行

找到 src/main/java/com/wechat/ferry/WeChatFerryApplication.java 类

直接启动即可

### 访问

启动后springboot自身的端口为 9201 socket的端口为 10086

swagger地址：http://localhost:9201/swagger-ui/index.html

## 参与开发

### 核心依赖

| 依赖            | 版本          | 说明       |
|---------------|-------------|----------|
| Spring Boot   | 2.7.18      | 基础框架     |
| protobuf-java | 3.22.2      | rpc      |
| jna           | 5.6.0       | 态访问系统本地库 |
| nng-java      | 1.4.0       | 本地包      |
| fastjson2     | 2.0.52      | 序列化      |
| dom4j         | 2.1.3       | XML解析包   |
| httpclient    | 4.5.13      | 客户端请求    |
| validation    | 2.0.1.Final | 参数校验     |
| springfox     | 3.0.0       | swagger3 |

### 模块结构

```
wechat-ferry-mvn
├─dll                                       核心dll
│  ├─sdk.dll                                sdk文件
│  └─readme.txt                             本目录说明文件
│ 
├─src                                       源
│  ├─main                                   重启命令
│  │  ├─java(com.wechat.ferry)              java代码目录
│  │  │  ├─config                           配置
│  │  │  ├─constant                         常量
│  │  │  ├─controller                       控制层(API接口)
│  │  │  ├─entity                           聚合模型
│  │  │  │  ├─dto                           DTO模型
│  │  │  │  ├─po                            数据库实体(与表结构一一对应,否则请使用DTO)
│  │  │  │  ├─proto                         PB实体
│  │  │  │  └─vo                            视图层返回体目录
│  │  │  ├─enums                            枚举
│  │  │  ├─exception                        异常封装
│  │  │  ├─handle                           处理层
│  │  │  ├─service                          业务层
│  │  │  │  └─impl                          业务实现类
│  │  │  ├─strategy                         策略层
│  │  │  │  └─impl                          策略实现类(如接收到消息之后的事件处理可以放在这里)
│  │  │  ├─task                             定时任务
│  │  │  ├─utils                            工具层
│  │  │  └─WcferryApplication.java          启动类
│  │  │
│  │  │resources                            资源目录
│  │  │  ├─libs                             本程序内置依赖包
│  │  │  ├─proto                            proto文件(此目录打包将被排除)
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

本程序内置参数统一前缀：wechat.ferry 所有自定义本服务的参数请都放置在此前缀下，如：

```ymal
wechat:
  ferry:
    # DLL文件位置
    dll-path: /dll/sdk.dll
```

### 缩写含义

首先特别通用的缩写，这里就不在重复了，比如DTO,PO这类

请求入参：req

请求出参：resp

微信：wx

个人微信：pp

企业微信：cp

微信公众号：mp

WeChatFerry框架：wcf

.gitkeep：占位文件，比如我们定义一个文件夹之后，为了避免文件夹下无内容，导致git未提交上，所以会建议此文件，阅读时直接忽略此类文件即可

### 生成proto文件

本程序已经集成了生成proto文件的maven插件，直接install即可生成proto文件，且会在打包程序中去除 src/main/resources/proto
下面的内容，只保留实体类中的文件

默认install会重新根据.proto文件重新生成实体，如果不想被替换，请删除 src/main/resources/proto 下对应的.proto文件即可

#### 本项目自适配类型

有一些查询数据的自定义返回字段，定义了本项目自身的返回类型，所以 proto 文件不要从其他项目拷贝，不然缺失类型导致接口返回乱码自行负责

如：

```
message RoomData

```

### 提交规范

本模块希望大家使用统一提交格式，便于区分

格式：类型(任务号/缺陷号/没有使用0替代): [模块名称]-[子模块名称]-本次修改的说明

如：

```cmd
feat(0): [java]-[wechat-ferry-mvn]-基础类目录划分迁移及代码格式
```

| 名称   | 版本           |
|------|--------------|
| feat | 新功能          |
| fix  | 缺陷           |
| ...  | 其他等git规范中的均可 |

### 说明

下面主要是针对一些合作开发者的咨询做统一回复，可以直接忽略。

#### 目录层级拆分太细

其实这个服务是底层服务，目录分细确实会增加开发量，但是针对于后面的拓展和维护上来说，个人感觉利大于弊，
毕竟功能开发完之后，这个服务很少在变动，更多的是阅读，如果能让大家阅读、维护方便，开发时多花点时间能接受。

当然，如果觉得目录太多，可以自己删除点，这样可以少写点代码，目前本项目还是建议保必要的层级隔离

#### 文件名太长

出发点是为了名称进行统一规范化，便于阅读更加方便，只要了解了基础的缩写含义，其实跟自己平时命名一样，
只不过加上了固定的前缀和后缀而已，这样单从文件名就能分析出功能模块及其含义
