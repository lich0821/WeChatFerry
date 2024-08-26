package httpd

import (
	"github.com/opentdp/go-helper/httpd"

	"github.com/opentdp/wrest-chat/args"
	"github.com/opentdp/wrest-chat/httpd/middle"
	"github.com/opentdp/wrest-chat/httpd/wcfrest"
)

// @title Wrest Chat Api
// @version v0.10.0
// @description 基于 WeChatFerry RPC 实现的微信接口，使用 Go 语言编写，无第三方运行时依赖，易于对接任意编程语言。
// @contact.name WeChatRest
// @contact.url https://github.com/opentdp/wrest-chat
// @license.name Apache 2.0
// @license.url http://www.apache.org/licenses/LICENSE-2.0.html
// @BasePath /

func Server() {

	httpd.Engine(args.Debug)

	// Wcfrest 路由
	wcfrest.Route()

	// Swagger 守卫
	httpd.Use(middle.SwaggerGuard)

	// 前端文件路由
	httpd.StaticEmbed("/", "public", args.Efs)

	// 启动 HTTP 服务
	httpd.Server(args.Web.Address)

}
