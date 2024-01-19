package wcfrest

import (
	"github.com/gin-gonic/gin"

	"github.com/opentdp/wechat-rest/wclient"
)

func Route(rg *gin.RouterGroup) {

	ctrl := &Controller{wclient.Register()}

	rg.POST("is_login", ctrl.isLogin)
	rg.POST("self_wxid", ctrl.getSelfWxid)
	rg.POST("self_info", ctrl.getSelfInfo)
	rg.POST("msg_types", ctrl.getMsgTypes)

	rg.POST("db_names", ctrl.getDbNames)
	rg.POST("db_tables", ctrl.getDbTables)
	rg.POST("db_query_sql", ctrl.dbSqlQuery)

	rg.POST("chatrooms", ctrl.getChatRooms)
	rg.POST("chatroom_members", ctrl.getChatRoomMembers)
	rg.POST("alias_in_chatroom", ctrl.getAliasInChatRoom)
	rg.POST("invite_chatroom_members", ctrl.inviteChatroomMembers)
	rg.POST("add_chatroom_members", ctrl.addChatRoomMembers)
	rg.POST("del_chatroom_members", ctrl.delChatRoomMembers)

	rg.POST("revoke_msg", ctrl.revokeMsg)
	rg.POST("forward_msg", ctrl.forwardMsg)
	rg.POST("send_txt", ctrl.sendTxt)
	rg.POST("send_img", ctrl.sendImg)
	rg.POST("send_file", ctrl.sendFile)
	rg.POST("send_rich_text", ctrl.sendRichText)
	rg.POST("send_pat_msg", ctrl.sendPatMsg)
	rg.POST("audio_msg", ctrl.getAudioMsg)
	rg.POST("ocr_result", ctrl.getOcrResult)
	rg.POST("download_image", ctrl.downloadImage)
	rg.POST("download_attach", ctrl.downloadAttach)

	rg.POST("avatars", ctrl.getAvatars)
	rg.POST("contacts", ctrl.getContacts)
	rg.POST("friends", ctrl.getFriends)
	rg.POST("user_info", ctrl.getInfoByWxid)
	rg.POST("refresh_pyq", ctrl.refreshPyq)
	rg.POST("accept_new_friend", ctrl.acceptNewFriend)
	rg.POST("receive_transfer", ctrl.receiveTransfer)

	rg.POST("enable_receiver", ctrl.enabledReceiver)
	rg.POST("disable_receiver", ctrl.disableReceiver)

	rg.GET("socket_receiver", ctrl.socketReceiver)

}
