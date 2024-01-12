package wcfrest

import (
	"github.com/gin-gonic/gin"
	"github.com/opentdp/wechat-rest/wclient"
)

func Route(rg *gin.RouterGroup) {

	ctrl := Controller{wclient.Register()}

	rg.GET("is_login", ctrl.isLogin)
	rg.GET("self_wxid", ctrl.getSelfWxid)
	rg.GET("user_info", ctrl.getUserInfo)
	rg.GET("contacts", ctrl.getContacts)
	rg.GET("friends", ctrl.getFriends)
	rg.GET("user_info/:wxid", ctrl.getUserInfoByWxid)

	rg.GET("db_names", ctrl.getDbNames)
	rg.GET("db_tables/:db", ctrl.getDbTables)
	rg.POST("db_query_sql", ctrl.dbSqlQuery)

	rg.GET("msg_types", ctrl.getMsgTypes)
	rg.GET("refresh_pyq/:id", ctrl.refreshPyq)

	rg.GET("chatrooms", ctrl.getChatRooms)
	rg.GET("chatroom_members/:roomid", ctrl.getChatRoomMembers)
	rg.GET("alias_in_chatroom/:wxid/:roomid", ctrl.getAliasInChatRoom)
	rg.POST("invite_chatroom_members", ctrl.inviteChatroomMembers)
	rg.POST("add_chatroom_members", ctrl.addChatRoomMembers)
	rg.POST("del_chatroom_members", ctrl.delChatRoomMembers)

	rg.GET("revoke_msg/:msgid", ctrl.revokeMsg)
	rg.POST("forward_msg", ctrl.forwardMsg)
	rg.POST("send_txt", ctrl.sendTxt)
	rg.POST("send_img", ctrl.sendImg)
	rg.POST("send_file", ctrl.sendFile)
	rg.POST("send_rich_text", ctrl.sendRichText)
	rg.POST("send_pat_msg", ctrl.sendPatMsg)
	rg.POST("get_audio_msg", ctrl.getAudioMsg)
	rg.POST("get_ocr_result", ctrl.getOcrResult)
	rg.POST("download_image", ctrl.downloadImage)
	rg.POST("download_attach", ctrl.downloadAttach)

	rg.POST("accept_new_friend", ctrl.acceptNewFriend)
	rg.POST("receive_transfer", ctrl.receiveTransfer)

	rg.POST("enable_receiver", ctrl.enabledReceiver)
	rg.POST("disable_receiver", ctrl.disableReceiver)

}
