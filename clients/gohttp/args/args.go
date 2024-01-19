package args

// 日志配置

var Log = struct {
	Dir    string `yaml:"dir"`
	Level  string `yaml:"level"`
	Target string `yaml:"target"`
}{
	Dir:    "logs",
	Level:  "info",
	Target: "stdout",
}

// Web 服务

var Web = struct {
	Address string `yaml:"address"`
	Swagger bool   `yaml:"swagger"`
	Token   string `yaml:"token"`
}{
	Address: "127.0.0.1:7600",
	Swagger: true,
}

// Wcf 服务

var Wcf = struct {
	Address    string `yaml:"address"`
	WeChatAuto bool   `yaml:"wechatAuto"`
	MsgPrinter bool   `yaml:"msgPrinter"`
}{
	Address:    "127.0.0.1:7601",
	WeChatAuto: true,
}
