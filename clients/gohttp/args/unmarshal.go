package args

import (
	"os"

	"github.com/knadh/koanf/providers/confmap"
	"github.com/opentdp/go-helper/logman"
)

func (c *Config) Unmarshal() {

	// 读取默认配置

	mp := map[string]any{
		"log": &Log,
		"web": &Web,
		"wcf": &Wcf,
	}
	c.Koanf.Load(confmap.Provider(mp, "."), nil)

	// 读取配置文件

	c.ReadYaml()
	for k, v := range mp {
		c.Koanf.Unmarshal(k, v)
	}

	// 初始化日志

	if Log.Dir != "" && Log.Dir != "." {
		os.MkdirAll(Log.Dir, 0755)
	}

	logman.SetDefault(&logman.Config{
		Level:    Log.Level,
		Target:   Log.Target,
		Storage:  Log.Dir,
		Filename: "wrest",
	})

	// 写入配置文件

	c.WriteYaml()

}
