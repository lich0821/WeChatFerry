package args

import (
	"os"

	"github.com/knadh/koanf/providers/confmap"
	"github.com/opentdp/go-helper/logman"
)

func (c *Config) Unmarshal() {

	// 读取默认配置

	mp := map[string]any{
		"httpd":  &Httpd,
		"logger": &Logger,
		"wcf":    &Wcf,
	}
	c.Koanf.Load(confmap.Provider(mp, "."), nil)

	// 读取配置文件

	c.ReadYaml()
	for k, v := range mp {
		c.Koanf.Unmarshal(k, v)
	}

	// 初始化日志

	if Logger.Dir != "" && Logger.Dir != "." {
		os.MkdirAll(Logger.Dir, 0755)
	}

	logman.SetDefault(&logman.Config{
		Level:    Logger.Level,
		Target:   Logger.Target,
		Storage:  Logger.Dir,
		Filename: "wrest",
	})

	// 写入配置文件

	c.WriteYaml()

}
