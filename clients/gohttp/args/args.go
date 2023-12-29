package args

import (
	"embed"
)

// 调试模式

var Debug bool

// 嵌入目录

var Efs *embed.FS

// 日志参数

var Logger = struct {
	Dir    string
	Level  string
	Target string
}{
	Dir:    "logs",
	Level:  "info",
	Target: "stdout",
}

// Http 服务参数

var Httpd = struct {
	Address string
	Token   string
	Swag    bool
}{
	Address: "127.0.0.1:7600",
	Swag:    true,
}

// Wcf 服务参数

var Wcf = struct {
	Address    string
	SdkLibrary string
	WeChatAuto bool
	MsgPrint   bool
}{
	Address:    "127.0.0.1:10080",
	SdkLibrary: "sdk.dll",
}
