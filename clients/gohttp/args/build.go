package args

import (
	"embed"
)

// 调试模式

var Debug bool

// 嵌入目录

var Efs *embed.FS

// 版本信息

const Version = "0.10.0"
const BuildVersion = "240106"

// 应用描述

const AppName = "TDP Wrest"
const AppSummary = "智能聊天机器人"

// 输出说明
func init() {

	println(AppName, AppSummary)
	println("Version:", Version, "build", BuildVersion)

}
