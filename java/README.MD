# WeChatFerry Java 客户端
## 配置环境
选择 32 位的 JDK [temurin](https://adoptium.net/zh-CN/temurin/releases)。

## 重新生成 gRPC 文件
[gRPC 文件生成工具](https://repo.maven.apache.org/maven2/io/grpc/protoc-gen-grpc-java/)。
```sh
cd java/src/main/java/
protoc --java_out=. -I=../../../../proto wcf.proto
protoc --grpc-java_out=. --plugin=protoc-gen-grpc-java="C:/Tools/bin/protoc-gen-grpc-java-1.49.2-windows-x86_32.exe" -I=../../../../proto wcf.proto
```

`C:/Tools/bin` 为工具存放路径。
