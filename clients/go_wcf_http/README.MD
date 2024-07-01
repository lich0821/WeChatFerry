# wechatFerry 的 go版本http端

接口文档：https://apifox.com/apidoc/shared-6e6950ec-1a6d-4545-90d6-d27d31af2b7c

http服务器的端口是 8000 需要修改的自行编译懒得写配置文件
localhost:8000是本地接口文档

由于用到了cgo 编译需要安装Mingw
Mingw下载地址：https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/mingw-builds/8.1.0/threads-win32/sjlj/x86_64-8.1.0-release-win32-sjlj-rt_v6-rev0.7z/download

打包命令

#x86 win 编译
set GOOS=windows
set GOARCH=amd64
go build -ldflags="-s -w" -o go_wcf_http3.9.10.27.exe .\main.go
