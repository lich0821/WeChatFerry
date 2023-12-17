package midware

import (
	"errors"

	"github.com/gin-gonic/gin"
)

// 获取错误代码

func exitCode(c *gin.Context, code int) int {

	if code := c.GetInt("ExitCode"); code > 100 {
		return code
	}

	return code

}

// 创建错误实例

func newError(data any) error {

	if err, ok := data.(error); ok {
		return err
	}

	if err, ok := data.(string); ok {
		return errors.New(err)
	}

	return errors.New("未知错误")

}

// 构造错误信息

func newErrorMessage(data any) gin.H {

	if err, ok := data.(error); ok {
		return gin.H{"Error": gin.H{"Message": err.Error()}}
	}

	if err, ok := data.(string); ok {
		return gin.H{"Error": gin.H{"Message": err}}
	}

	return gin.H{"Error": data}

}

// 构造结构数据

func newPayload(data any, msg, token string) gin.H {

	payload := gin.H{"Payload": data}

	if msg != "" {
		payload["Message"] = msg
	}

	if token != "" {
		payload["Token"] = token
	}

	return payload

}
