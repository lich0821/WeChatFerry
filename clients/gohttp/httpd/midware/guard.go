package midware

import (
	"strings"

	"github.com/gin-gonic/gin"

	"wechat-rest/args"
)

func ApiGuard(c *gin.Context) {

	token := ""

	authcode := c.GetHeader("Authorization")
	parts := strings.SplitN(authcode, " ", 2)
	if len(parts) == 2 && parts[0] == "Bearer" {
		token = parts[1]
	}

	if token != args.Httpd.Token {
		c.Set("Error", gin.H{"Code": 401, "Message": "操作未授权"})
		c.Set("ExitCode", 401)
		c.Abort()
	}

}

func SwagGuard(c *gin.Context) {

	if !args.Httpd.Swag && strings.HasPrefix(c.Request.URL.Path, "/swag") {
		c.Header("Content-Type", "text/html; charset=utf-8")
		c.String(200, "功能已禁用")
		c.Abort()
	}

}
