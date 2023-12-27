package wcfrest

import (
	"github.com/gin-gonic/gin"
)

func Route(rg *gin.RouterGroup) {

	initService()

	rg.GET("is_login", isLogin)
	rg.GET("self_wxid", getSelfWxid)
	rg.GET("user_info", getUserInfo)
	rg.GET("contacts", getContacts)
	rg.GET("friends", getFriends)
	rg.GET("user_info/:wxid", getUserInfoByWxid)

	rg.GET("db_names", getDbNames)
	rg.GET("db_tables/:db", getDbTables)
	rg.POST("db_query_sql", dbSqlQuery)

	rg.GET("msg_types", getMsgTypes)
	rg.GET("refresh_pyq/:id", refreshPyq)

	rg.GET("chatrooms", getChatRooms)
	rg.GET("chatroom_members/:roomid", getChatRoomMembers)
	rg.GET("alias_in_chatroom/:wxid/:roomid", getAliasInChatRoom)
	rg.POST("invite_chatroom_members", inviteChatroomMembers)
	rg.POST("add_chatroom_members", addChatRoomMembers)
	rg.POST("del_chatroom_members", delChatRoomMembers)

	rg.GET("revoke_msg/:msgid", revokeMsg)
	rg.POST("forward_msg", forwardMsg)
	rg.POST("send_txt", sendTxt)
	rg.POST("send_img", sendImg)
	rg.POST("send_file", sendFile)
	rg.POST("send_rich_text", sendRichText)
	rg.POST("send_pat_msg", sendPatMsg)
	rg.POST("get_audio_msg", getAudioMsg)
	rg.POST("get_ocr_result", getOcrResult)
	rg.POST("download_image", downloadImage)
	rg.POST("download_attach", downloadAttach)

	rg.POST("accept_new_friend", acceptNewFriend)
	rg.POST("receive_transfer", receiveTransfer)

	rg.POST("enable_forward_msg", enableForwardMsg)
	rg.POST("disable_forward_msg", disableForwardMsg)

}
