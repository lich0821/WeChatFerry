# WeChatFerry Python 客户端
代码于 `Python3.7` 环境开发。

## 配置环境
```sh
# 创建虚拟环境
python -m venv .env
# 激活虚拟环境
source .env/Scripts/activate
# 升级 pip
pip install --upgrade pip
# 安装依赖包
pip install grpcio grpcio-tools
```

## 运行
```sh
# 启动客户端
python demo.py
```

## 重新生成 gRPC 文件
```sh
cd wcf
python -m grpc_tools.protoc --python_out=. --grpc_python_out=. -I=../ wcf.proto
```
