package main

import (
	"embed"

	"wechat-rest/httpd"

	"github.com/opentdp/wrest-chat/args"
)

//go:embed public
var efs embed.FS

func main() {

	args.Efs = &efs

	httpd.Server()

}
