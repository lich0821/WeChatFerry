package midware

import (
	"strings"

	"github.com/gin-gonic/gin"

	"wechat-rest/args"
)

func ApiGuard(c *gin.Context) {

	token := ""

	// 取回 Token
	authcode := c.GetHeader("Authorization")
	parts := strings.SplitN(authcode, " ", 2)
	if len(parts) == 2 && parts[0] == "Bearer" {
		token = parts[1]
	}

	// 校验 Token
	if token != args.Web.Token {
		c.Set("Error", gin.H{"Code": 401, "Message": "操作未授权"})
		c.Set("ExitCode", 401)
		c.Abort()
	}

}

func SwaggerGuard(c *gin.Context) {

	if !args.Web.Swagger && strings.HasPrefix(c.Request.URL.Path, "/swagger") {
		c.Header("Content-Type", "text/html; charset=utf-8")
		c.String(200, "功能已禁用")
		c.Abort()
	}

}
