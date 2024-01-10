package args

import (
	"os"

	"github.com/knadh/koanf/parsers/yaml"
	"github.com/knadh/koanf/providers/file"
	"github.com/knadh/koanf/v2"
	"github.com/opentdp/go-helper/filer"
	"github.com/opentdp/go-helper/logman"
)

// 配置信息操作类

type Config struct {
	Koanf  *koanf.Koanf
	Parser *yaml.YAML
	File   string
}

func (c *Config) Init() *Config {

	debug := os.Getenv("TDP_DEBUG")
	Debug = debug == "1" || debug == "true"

	c.Koanf = koanf.NewWithConf(koanf.Conf{
		StrictMerge: true,
		Delim:       ".",
	})
	c.Parser = yaml.Parser()

	c.File = "config.yml"
	if len(os.Args) > 1 {
		c.File = os.Args[1]
	}

	return c

}

func (c *Config) ReadYaml() {

	// 配置不存在则忽略
	_, err := os.Stat(c.File)
	if os.IsNotExist(err) {
		return
	}

	// 从配置文件读取参数
	err = c.Koanf.Load(file.Provider(c.File), c.Parser)
	if err != nil {
		logman.Fatal("read config error", "error", err)
	}

}

func (c *Config) WriteYaml() {

	// 是否强制覆盖
	if filer.Exists(c.File) {
		return
	}

	// 序列化参数信息
	buf, err := c.Koanf.Marshal(c.Parser)
	if err != nil {
		logman.Fatal("write config error", "error", err)
	}

	// 将参数写入配置文件
	err = os.WriteFile(c.File, buf, 0644)
	if err != nil {
		logman.Fatal("write config error", "error", err)
	}

}
