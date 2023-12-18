package main

import (
	"embed"

	"wechat-rest/args"
	"wechat-rest/httpd"
)

//go:embed public
var efs embed.FS

func main() {

	args.Efs = &efs

	c := args.Config{}
	c.Init().Unmarshal()

	httpd.Server()

}
