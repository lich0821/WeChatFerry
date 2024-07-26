package main

import (
	"encoding/json"
	"fmt"
	"github.com/gin-gonic/gin"
	resty "github.com/go-resty/resty/v2"
	"github.com/gorilla/websocket"
	"go_wechatFerry/app"
	"go_wechatFerry/wcf"
	"log"
	"net"
	"net/http"
	"os"
	"os/signal"
	"strings"
	"time"
)

func init() {
	// 运行sdk.dll中的函数
	app.WechatFerryInit()
}

func httpInit() {
	// 1.创建路由
	r := gin.New()
	gin.SetMode(gin.ReleaseMode)
	// 2.绑定路由规则，执行的函数
	// gin.Context，封装了request和response
	// 设置模板目录
	r.LoadHTMLGlob("templates/*")
	r.GET("/", func(c *gin.Context) {
		c.HTML(http.StatusOK, "wechatFerryGoHttp.html", gin.H{})
	})

	r.POST("/testHttp", func(c *gin.Context) {
		type RequestData struct {
			Code    int    `json:"code"`
			Message string `json:"message"`
			Data    struct {
				IsGroup   bool   `json:"is_group"`
				MessageId uint64 `json:"message_id"`
				Type      uint32 `json:"type"`
				Ts        uint32 `json:"ts"`
				RoomId    string `json:"room_id"`
				Content   string `json:"content"`
				WxId      string `json:"wx_id"`
				Sign      string `json:"sign"`
				Xml       string `json:"xml"`
			} `json:"data"`
		}
		var requestData RequestData
		c.BindJSON(&requestData)
		fmt.Println(requestData)
	})

	// 设置消息回调地址
	r.POST("/api/SetMessageCallbackUrl", app.SetMessageCallbackUrl)
	// 获取登录的wx_id
	r.GET("/api/GetSelfWXID", app.GetSelfWXID)
	// 获取自己的信息
	r.GET("/api/GetUserInfo", app.GetUserInfo)
	// 获取消息类型列表
	r.GET("/api/GetMsgTypes", app.GetMsgTypes)
	// 获取通讯录成功
	r.GET("/api/GetContacts", app.GetContacts)
	// 获取全部群的群成员
	r.GET("/api/GetRoomMembersAll", app.GetRoomMembersAll)
	// 获取单个群成员列表
	r.POST("/api/GetRoomMember", app.GetRoomMember)
	// 获取数据库名
	r.GET("/api/GetDBNames", app.GetDBNames)
	// 获取表
	r.POST("/api/GetDBTables", app.GetDBTables)
	// 执行sql
	r.POST("/api/ExecDBQuery", app.ExecDBQuery)
	// 发送文本消息
	r.POST("/api/SendTxt", app.SendTxt)
	// 发送图片
	r.POST("/api/SendIMG", app.SendIMG)
	// 发送文件
	r.POST("/api/SendFile", app.SendFile)
	// 发送卡片消息
	r.POST("/api/SendRichText", app.SendRichText)
	// 发送拍一拍消息
	r.POST("/api/SendPat", app.SendPat)
	// 转发消息
	r.POST("/api/ForwardMsg", app.ForwardMsg)

	// 发送emoji消息
	r.POST("/api/SendEmotion", app.SendEmotion)
	// 接受好友请求
	r.POST("/api/AcceptFriend", app.AcceptFriend)
	// 添加群成员
	r.POST("/api/AddChatroomMembers", app.AddChatroomMembers)
	// 邀请群成员
	r.POST("/api/InvChatRoomMembers", app.InvChatRoomMembers)
	// 删除群成员
	r.POST("/api/DelChatRoomMembers", app.DelChatRoomMembers)
	// 刷新朋友圈
	r.POST("/api/RefreshPyq", app.RefreshPyq)
	// 下载附件
	r.POST("/api/DownloadAttach", app.DownloadAttach)

	r.Run("127.0.0.1:8000")
}

func OnMsg() {
	err := app.WxClient.OnMSG(func(msg *wcf.WxMsg) {
		var message app.Message
		message.IsSelf = msg.IsSelf
		message.IsGroup = msg.IsGroup
		message.MessageId = msg.Id
		message.Type = msg.Type
		message.Ts = msg.Ts
		message.RoomId = msg.Roomid
		message.Content = msg.Content
		message.Sign = msg.Sign
		message.WxId = msg.Sender
		message.Thumb = msg.Thumb
		message.Extra = msg.Extra
		message.Xml = msg.Xml
		// 如果你设置了回调链接 那么他就是会传给你 如果你没设置 你可以在else中 添加你的代码 直接删掉 回调的判断即可
		if app.WxClient.MessageCallbackUrl != "" {
			var data = map[string]interface{}{
				"code":    0,
				"message": "微信消息",
				"data":    message,
			}
			jsonData, _ := json.Marshal(data)
			if strings.Contains(app.WxClient.MessageCallbackUrl, "tcp://") {
				conn, err := net.Dial("tcp", app.WxClient.MessageCallbackUrl)
				defer conn.Close()
				if err != nil {
					fmt.Println("err :", err)
				} else {
					_, err := conn.Write(jsonData)
					fmt.Println("err :", err)
				}
			}
			if strings.Contains(app.WxClient.MessageCallbackUrl, "udp://") {
				addr, err := net.ResolveUDPAddr("udp", app.WxClient.MessageCallbackUrl)
				if err != nil {
					fmt.Println("Error resolving address:", err)
					return
				}
				// 创建 UDP 连接
				conn, err := net.DialUDP("udp", nil, addr)
				if err != nil {
					fmt.Println("Error dialing:", err)
					return
				}
				defer conn.Close()
				// 发送消息
				_, err = conn.Write(jsonData)
				if err != nil {
					fmt.Println("Error sending message:", err)
					return
				}
			}
			if strings.Contains(app.WxClient.MessageCallbackUrl, "ws://") {
				// 创建 WebSocket 连接
				conn, _, err := websocket.DefaultDialer.Dial(app.WxClient.MessageCallbackUrl, nil)
				if err != nil {
					log.Fatal("Dial error:", err)
				}
				defer conn.Close()
				// 设置写入超时
				conn.SetWriteDeadline(time.Now().Add(5 * time.Second))
				// 发送消息
				err = conn.WriteMessage(websocket.TextMessage, jsonData)
				if err != nil {
					log.Fatal("Write error:", err)
				}
			}
			if strings.Contains(app.WxClient.MessageCallbackUrl, "http") {
				_, err := resty.New().SetTimeout(5 * time.Second).R().SetBody(jsonData).Post(app.WxClient.MessageCallbackUrl)
				if err != nil {
					fmt.Println("http消息发送失败")
				}
			}
		} else {
			//fmt.Println("消息类型：", message.Type)
			//fmt.Println("消息Thumb：", message.Thumb) // 这个可以直接下载
			//fmt.Println("消息Extra：", message.Extra) // 这个要点一下才能下载(自行处理)
			//fmt.Println("消息xml：", message.Xml)
			//if message.Type == 3 {
			//	resp, _ := resty.New().R().SetBody(map[string]interface{}{
			//		"id":    message.MessageId,
			//		"thumb": "F:/c++/WeChatFerry/clients/go_wcf_http/", //下载到本地的哪里
			//		"extra": message.Thumb,                             // 看上面的
			//	}).Post("http://127.0.0.1:8001/api/DownloadAttach")
			//	fmt.Println(resp.String())
			//	fmt.Println(resp.Error())
			//	fmt.Println(resp.StatusCode())
			//}
			go app.MessageProcess(message)
		}
	})
	if err != nil {
		fmt.Println(err)
		OnMsg()
	} else {
		fmt.Println("为正常接受消息状态")
	}
}

// 入口
func main() {
	// 注册Ctrl+C信号处理函数
	signalChan := make(chan os.Signal, 1)
	signal.Notify(signalChan, os.Interrupt)
	go func() {
		<-signalChan
		// 在收到Ctrl+C信号时执行清理操作
		fmt.Println("\n感谢温柔的ctrl+c关闭，下次可直接运行程序，无需重启微信。")
		app.WxClient.Close()
		os.Exit(0)
	}()
	// 开启接收消息
	_ = app.WxClient.EnableRecvTxt()
	// 先启动http服务器 下面的会阻塞
	go httpInit()
	// 启动推送消息的地方
	go OnMsg()
	// 防止主goroutine退出
	select {}
}
