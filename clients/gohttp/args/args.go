package args

import (
	"embed"
)

// 调试模式

var Debug bool

// 嵌入目录

var Efs *embed.FS

// Http 服务参数

var Httpd = struct {
	Address string `yaml:"address"`
	Token   string `yaml:"token"`
	Swag    bool   `yaml:"swag"`
}{
	Address: "127.0.0.1:7600",
	Swag:    true,
}

// 日志参数

var Logger = struct {
	Dir    string `yaml:"dir"`
	Level  string `yaml:"level"`
	Target string `yaml:"target"`
}{
	Dir:    "logs",
	Level:  "info",
	Target: "stdout",
}

// Wcf 服务参数

var Wcf = struct {
	Address    string `yaml:"address"`
	SdkLibrary string `yaml:"sdkLibrary"`
	WeChatAuto bool   `yaml:"wechatAuto"`
	MsgPrinter bool   `yaml:"msgPrinter"`
}{
	Address:    "127.0.0.1:10080",
	SdkLibrary: "sdk.dll",
}
