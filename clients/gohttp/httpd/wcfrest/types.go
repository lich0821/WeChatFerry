package wcfrest

// 执行结果
type RespPayload struct {
	Success bool   `json:"success,omitempty"`
	Result  string `json:"result,omitempty"`
	Error   error  `json:"error,omitempty"`
}

// 数据库查询参数
type DbSqlQueryRequest struct {
	Db  string `json:"db"`
	Sql string `json:"sql"`
}

// 消息转发参数
type ForwardMsgRequest struct {
	Url string `json:"url"`
}

// 获取音频消息参数
type GetAudioMsgRequest struct {
	Msgid   uint64 `json:"msgid"`
	Dir     string `json:"path"`
	Timeout int    `json:"timeout"`
}

// 获取OCR识别参数
type GetOcrRequest struct {
	Extra   string `json:"extra"`
	Timeout int    `json:"timeout"`
}

// 下载图片参数
type DownloadImageRequest struct {
	Msgid   uint64 `json:"msgid"`
	Extra   string `json:"extra"`
	Dir     string `json:"dir"`
	Timeout int    `json:"timeout"`
}

// 下载附件参数
type DownloadAttachRequest struct {
	Msgid uint64 `json:"msgid"`
	Thumb string `json:"thumb"`
	Extra string `json:"extra"`
}
