package wcfrest

import (
	"strings"

	"github.com/gin-gonic/gin"
	"github.com/opentdp/go-helper/logman"
	"github.com/opentdp/go-helper/request"
	"github.com/opentdp/go-helper/strutil"

	"github.com/opentdp/wechat-rest/args"
	"github.com/opentdp/wechat-rest/wcferry"
)

var wc *wcferry.Client

func initService() {

	parts := strings.Split(args.Wcf.Address, ":")

	wc = &wcferry.Client{
		ListenAddr: parts[0],
		ListenPort: strutil.ToInt(parts[1]),
		SdkLibrary: args.Wcf.SdkLibrary,
		WeChatAuto: args.Wcf.WeChatAuto,
	}

	logman.Info("wcf starting ...")
	if err := wc.Connect(); err != nil {
		logman.Fatal("failed to start wcf", "error", err)
	}

	// 打印收到的消息
	if args.Wcf.MsgPrint {
		wc.EnrollReceiver(true, wcferry.MsgPrinter)
	}

}

// @Summary 检查登录状态
// @Produce json
// @Success 200 {object} bool
// @Router /is_login [get]
func isLogin(c *gin.Context) {

	c.Set("Payload", wc.CmdClient.IsLogin())

}

// @Summary 获取登录账号wxid
// @Produce json
// @Success 200 {object} string
// @Router /self_wxid [get]
func getSelfWxid(c *gin.Context) {

	c.Set("Payload", wc.CmdClient.GetSelfWxid())

}

// @Summary 获取登录账号个人信息
// @Produce json
// @Success 200 {object} wcferry.UserInfo
// @Router /user_info [get]
func getUserInfo(c *gin.Context) {

	c.Set("Payload", wc.CmdClient.GetUserInfo())

}

// @Summary 获取完整通讯录
// @Produce json
// @Success 200 {object} []wcferry.RpcContact
// @Router /contacts [get]
func getContacts(c *gin.Context) {

	c.Set("Payload", wc.CmdClient.GetContacts())

}

// @Summary 获取好友列表
// @Produce json
// @Success 200 {object} []wcferry.RpcContact
// @Router /friends [get]
func getFriends(c *gin.Context) {

	c.Set("Payload", wc.CmdClient.GetFriends())

}

// @Summary 根据wxid获取个人信息
// @Produce json
// @Param wxid path string true "wxid"
// @Success 200 {object} wcferry.RpcContact
// @Router /user_info/{wxid} [get]
func getUserInfoByWxid(c *gin.Context) {

	wxid := c.Param("wxid")
	c.Set("Payload", wc.CmdClient.GetInfoByWxid(wxid))

}

// @Summary 获取数据库列表
// @Produce json
// @Success 200 {object} []string
// @Router /db_names [get]
func getDbNames(c *gin.Context) {

	c.Set("Payload", wc.CmdClient.GetDbNames())

}

// @Summary 获取数据库表列表
// @Produce json
// @Param db path string true "数据库名"
// @Success 200 {object} []wcferry.DbTable
// @Router /db_tables/{db} [get]
func getDbTables(c *gin.Context) {

	db := c.Param("db")
	c.Set("Payload", wc.CmdClient.GetDbTables(db))

}

// @Summary 执行数据库查询
// @Produce json
// @Param body body DbSqlQueryRequest true "数据库查询请求参数"
// @Success 200 {object} map[string]any
// @Router /db_query_sql [post]
func dbSqlQuery(c *gin.Context) {

	var req DbSqlQueryRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.Set("Error", err)
		return
	}

	c.Set("Payload", wc.CmdClient.DbSqlQueryMap(req.Db, req.Sql))

}

// @Summary 获取所有消息类型
// @Produce json
// @Success 200 {object} map[int32]string
// @Router /msg_types [get]
func getMsgTypes(c *gin.Context) {

	c.Set("Payload", wc.CmdClient.GetMsgTypes())

}

// @Summary 刷新朋友圈
// @Produce json
// @Param id path int true "朋友圈id"
// @Success 200 {object} RespPayload
// @Router /refresh_pyq/{id} [get]
func refreshPyq(c *gin.Context) {

	id := c.Param("id")
	pyqid := uint64(strutil.ToUint(id))

	status := wc.CmdClient.RefreshPyq(pyqid)

	c.Set("Payload", RespPayload{
		Success: status == 1,
	})

}

// @Summary 获取群列表
// @Produce json
// @Success 200 {object} []wcferry.RpcContact
// @Router /chatrooms [get]
func getChatRooms(c *gin.Context) {

	c.Set("Payload", wc.CmdClient.GetChatRooms())

}

// @Summary 获取群成员列表
// @Produce json
// @Param roomid path string true "群id"
// @Success 200 {object} []wcferry.RpcContact
// @Router /chatroom_members/{roomid} [get]
func getChatRoomMembers(c *gin.Context) {

	roomid := c.Param("roomid")
	c.Set("Payload", wc.CmdClient.GetChatRoomMembers(roomid))

}

// @Summary 获取群成员昵称
// @Produce json
// @Param wxid path string true "wxid"
// @Param roomid path string true "群id"
// @Success 200 {object} string
// @Router /alias_in_chatroom/{wxid}/{roomid} [get]
func getAliasInChatRoom(c *gin.Context) {

	wxid := c.Param("wxid")
	roomid := c.Param("roomid")
	c.Set("Payload", wc.CmdClient.GetAliasInChatRoom(wxid, roomid))

}

// @Summary 邀请群成员
// @Produce json
// @Param body body wcferry.MemberMgmt true "增删群成员请求参数"
// @Success 200 {object} RespPayload
// @Router /invite_chatroom_members [post]
func inviteChatroomMembers(c *gin.Context) {

	var req wcferry.MemberMgmt
	if err := c.ShouldBindJSON(&req); err != nil {
		c.Set("Error", err)
		return
	}

	status := wc.CmdClient.InviteChatroomMembers(req.Roomid, req.Wxids)

	c.Set("Payload", RespPayload{
		Success: status == 1,
	})

}

// @Summary 添加群成员
// @Produce json
// @Param body body wcferry.MemberMgmt true "增删群成员请求参数"
// @Success 200 {object} RespPayload
// @Router /add_chatroom_members [post]
func addChatRoomMembers(c *gin.Context) {

	var req wcferry.MemberMgmt
	if err := c.ShouldBindJSON(&req); err != nil {
		c.Set("Error", err)
		return
	}

	status := wc.CmdClient.AddChatRoomMembers(req.Roomid, req.Wxids)

	c.Set("Payload", RespPayload{
		Success: status == 1,
	})

}

// @Summary 删除群成员
// @Produce json
// @Param body body wcferry.MemberMgmt true "增删群成员请求参数"
// @Success 200 {object} RespPayload
// @Router /del_chatroom_members [post]
func delChatRoomMembers(c *gin.Context) {

	var req wcferry.MemberMgmt
	if err := c.ShouldBindJSON(&req); err != nil {
		c.Set("Error", err)
		return
	}

	status := wc.CmdClient.DelChatRoomMembers(req.Roomid, req.Wxids)

	c.Set("Payload", RespPayload{
		Success: status == 1,
	})

}

// @Summary 撤回消息
// @Produce json
// @Param msgid path int true "消息id"
// @Success 200 {object} RespPayload
// @Router /revoke_msg/{msgid} [get]
func revokeMsg(c *gin.Context) {

	id := c.Param("msgid")
	msgid := uint64(strutil.ToUint(id))

	status := wc.CmdClient.RevokeMsg(msgid)

	c.Set("Payload", RespPayload{
		Success: status == 1,
	})

}

// @Summary 发送文本消息
// @Produce json
// @Param body body wcferry.TextMsg true "文本消息请求参数"
// @Success 200 {object} RespPayload
// @Router /send_txt [post]
func sendTxt(c *gin.Context) {

	var req wcferry.TextMsg
	if err := c.ShouldBindJSON(&req); err != nil {
		c.Set("Error", err)
		return
	}

	status := wc.CmdClient.SendTxt(req.Msg, req.Receiver, req.Aters)

	c.Set("Payload", RespPayload{
		Success: status == 0,
	})

}

// @Summary 发送图片消息
// @Produce json
// @Param body body wcferry.PathMsg true "图片消息请求参数"
// @Success 200 {object} RespPayload
// @Router /send_img [post]
func sendImg(c *gin.Context) {

	var req wcferry.PathMsg
	if err := c.ShouldBindJSON(&req); err != nil {
		c.Set("Error", err)
		return
	}

	status := wc.CmdClient.SendImg(req.Path, req.Receiver)

	c.Set("Payload", RespPayload{
		Success: status == 0,
	})

}

// @Summary 发送文件消息
// @Produce json
// @Param body body wcferry.PathMsg true "文件消息请求参数"
// @Success 200 {object} RespPayload
// @Router /send_file [post]
func sendFile(c *gin.Context) {

	var req wcferry.PathMsg
	if err := c.ShouldBindJSON(&req); err != nil {
		c.Set("Error", err)
		return
	}

	status := wc.CmdClient.SendFile(req.Path, req.Receiver)

	c.Set("Payload", RespPayload{
		Success: status == 0,
	})

}

// @Summary 发送卡片消息
// @Produce json
// @Param body body wcferry.RichText true "卡片消息请求参数"
// @Success 200 {object} RespPayload
// @Router /send_rich_text [post]
func sendRichText(c *gin.Context) {

	var req wcferry.RichText
	if err := c.ShouldBindJSON(&req); err != nil {
		c.Set("Error", err)
		return
	}

	status := wc.CmdClient.SendRichText(req.Name, req.Account, req.Title, req.Digest, req.Url, req.Thumburl, req.Receiver)

	c.Set("Payload", RespPayload{
		Success: status == 0,
	})

}

// @Summary 拍一拍群友
// @Produce json
// @Param body body wcferry.PatMsg true "拍一拍请求参数"
// @Success 200 {object} RespPayload
// @Router /send_pat_msg [post]
func sendPatMsg(c *gin.Context) {

	var req wcferry.PatMsg
	if err := c.ShouldBindJSON(&req); err != nil {
		c.Set("Error", err)
		return
	}

	status := wc.CmdClient.SendPatMsg(req.Roomid, req.Wxid)

	c.Set("Payload", RespPayload{
		Success: status == 1,
	})

}

// @Summary 获取语音消息
// @Produce json
// @Param body body GetAudioMsgRequest true "语音消息请求参数"
// @Success 200 {object} RespPayload
// @Router /get_audio_msg [post]
func getAudioMsg(c *gin.Context) {

	var req GetAudioMsgRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.Set("Error", err)
		return
	}

	if req.Timeout > 0 {
		resp, err := wc.CmdClient.GetAudioMsgTimeout(req.Msgid, req.Dir, req.Timeout)
		c.Set("Payload", RespPayload{
			Success: resp != "",
			Result:  resp,
			Error:   err,
		})
	} else {
		resp := wc.CmdClient.GetAudioMsg(req.Msgid, req.Dir)
		c.Set("Payload", RespPayload{
			Success: resp != "",
			Result:  resp,
		})
	}

}

// @Summary 获取OCR识别结果
// @Produce json
// @Param body body GetOcrRequest true "文本请求参数"
// @Success 200 {object} RespPayload
// @Router /get_ocr_result [post]
func getOcrResult(c *gin.Context) {

	var req GetOcrRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.Set("Error", err)
		return
	}

	if req.Timeout > 0 {
		resp, err := wc.CmdClient.GetOcrResultTimeout(req.Extra, req.Timeout)
		c.Set("Payload", RespPayload{
			Success: resp != "",
			Result:  resp,
			Error:   err,
		})
	} else {
		resp, stat := wc.CmdClient.GetOcrResult(req.Extra)
		c.Set("Payload", RespPayload{
			Success: stat == 0,
			Result:  resp,
		})
	}

}

// @Summary 下载图片
// @Produce json
// @Param body body DownloadImageRequest true "下载图片参数"
// @Success 200 {object} RespPayload
// @Router /download_image [post]
func downloadImage(c *gin.Context) {

	var req DownloadImageRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.Set("Error", err)
		return
	}

	resp, err := wc.CmdClient.DownloadImage(req.Msgid, req.Extra, req.Dir, req.Timeout)

	c.Set("Payload", RespPayload{
		Success: resp != "",
		Result:  resp,
		Error:   err,
	})

}

// @Summary 下载附件
// @Produce json
// @Param body body DownloadAttachRequest true "下载附件参数"
// @Success 200 {object} RespPayload
// @Router /download_attach [post]
func downloadAttach(c *gin.Context) {

	var req DownloadAttachRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.Set("Error", err)
		return
	}

	status := wc.CmdClient.DownloadAttach(req.Msgid, req.Thumb, req.Extra)

	c.Set("Payload", RespPayload{
		Success: status == 0,
	})

}

// @Summary 接受好友请求
// @Produce json
// @Param body body wcferry.Verification true "接受好友请求参数"
// @Success 200 {object} RespPayload
// @Router /accept_new_friend [post]
func acceptNewFriend(c *gin.Context) {

	var req wcferry.Verification
	if err := c.ShouldBindJSON(&req); err != nil {
		c.Set("Error", err)
		return
	}

	status := wc.CmdClient.AcceptNewFriend(req.V3, req.V4, req.Scene)

	c.Set("Payload", RespPayload{
		Success: status == 1,
	})

}

// @Summary 接受转账
// @Produce json
// @Param body body wcferry.Transfer true "接受转账请求参数"
// @Success 200 {object} RespPayload
// @Router /receive_transfer [post]
func receiveTransfer(c *gin.Context) {

	var req wcferry.Transfer
	if err := c.ShouldBindJSON(&req); err != nil {
		c.Set("Error", err)
		return
	}

	status := wc.CmdClient.ReceiveTransfer(req.Wxid, req.Tfid, req.Taid)

	c.Set("Payload", RespPayload{
		Success: status == 1,
	})

}

// @Summary 开启消息转发
// @Produce json
// @Param body body ForwardMsgRequest true "消息转发请求参数"
// @Success 200 {object} RespPayload
// @Router /enable_forward_msg [post]
func enableForwardMsg(c *gin.Context) {

	var req ForwardMsgRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.Set("Error", err)
		return
	}

	if !strings.HasPrefix(req.Url, "http") {
		c.Set("Error", "url must start with http(s)://")
		return
	}

	err := wc.EnrollReceiver(true, func(msg *wcferry.WxMsg) {
		logman.Info("forward msg", "url", req.Url, "Id", msg.Id)
		request.JsonPost(req.Url, msg, request.H{})
	})

	c.Set("Payload", RespPayload{
		Success: err == nil,
		Error:   err,
	})

}

// @Summary 关闭消息转发
// @Produce json
// @Param body body ForwardMsgRequest true "消息转发请求参数"
// @Success 200 {object} RespPayload
// @Router /disable_forward_msg [post]
func disableForwardMsg(c *gin.Context) {

	err := wc.DisableReceiver()

	c.Set("Payload", RespPayload{
		Success: err == nil,
		Error:   err,
	})

}
