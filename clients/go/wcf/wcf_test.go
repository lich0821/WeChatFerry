package wcf

import (
	logs "github.com/danbai225/go-logs"
	"go.nanomsg.org/mangos/v3/protocol/pair1"
	_ "go.nanomsg.org/mangos/v3/transport/all"
	"google.golang.org/protobuf/proto"
	"testing"
)

func TestConn(t *testing.T) {
	socket, err := pair1.NewSocket()
	if err != nil {
		logs.Err(err)
		return
	}
	err = socket.Dial("tcp://127.0.0.1:10086")
	if err != nil {
		logs.Err(err)
		return
	}
	response := Response{
		Func: Functions_FUNC_IS_LOGIN,
		Msg:  nil,
	}
	marshal, err := proto.Marshal(&response)
	if err != nil {
		logs.Err(err)
		return
	}
	socket.Send(marshal)
	if err != nil {
		logs.Err(err)
		return
	}
	recv, err := socket.Recv()
	if err != nil {
		logs.Err(err)
		return
	}
	msg := Response{}
	err = proto.Unmarshal(recv, &msg)
	if err != nil {
		logs.Err(err)
		return
	}
	logs.Info(msg.GetStatus())

}
func TestIsLogin(t *testing.T) {
	wcf, err := NewWCF("")
	if err != nil {
		logs.Err(err)
		return
	}
	logs.Info(wcf.IsLogin())
}
func TestGetSelfWXID(t *testing.T) {
	wcf, err := NewWCF("")
	if err != nil {
		logs.Err(err)
		return
	}
	logs.Info(wcf.GetSelfWXID())
}
func TestGetMsgTypes(t *testing.T) {
	wcf, err := NewWCF("")
	if err != nil {
		logs.Err(err)
		return
	}
	logs.Info(wcf.GetMsgTypes())
}
func TestGetContacts(t *testing.T) {
	wcf, err := NewWCF("")
	if err != nil {
		logs.Err(err)
		return
	}
	for _, contact := range wcf.GetContacts() {
		logs.Info(contact.Remark, contact.Wxid)
	}
	logs.Info(wcf.GetContacts())
}
func TestGetDBNames(t *testing.T) {
	wcf, err := NewWCF("")
	if err != nil {
		logs.Err(err)
		return
	}
	logs.Info(wcf.GetDBNames())
}
func TestGetDBTables(t *testing.T) {
	wcf, err := NewWCF("")
	if err != nil {
		logs.Err(err)
		return
	}
	logs.Info(wcf.GetDBTables("ChatMsg.db"))
}
func TestExecDBQuery(t *testing.T) {
	wcf, err := NewWCF("")
	if err != nil {
		logs.Err(err)
		return
	}
	logs.Info(wcf.ExecDBQuery("ChatMsg.db", "SELECT * FROM Name2ID_v1"))
}
func TestAcceptFriend(t *testing.T) {
	wcf, err := NewWCF("")
	if err != nil {
		logs.Err(err)
		return
	}
	logs.Info(wcf.AcceptFriend("encryptusername", "ticket", 17))
}
func TestGetUserInfo(t *testing.T) {
	wcf, err := NewWCF("")
	if err != nil {
		logs.Err(err)
		return
	}
	logs.Info(wcf.GetUserInfo())
}
func TestSendTxT(t *testing.T) {
	wcf, err := NewWCF("")
	if err != nil {
		logs.Err(err)
		return
	}
	logs.Info(wcf.SendTxt(" Hello @ 淡白", "45415088466@chatroom", []string{"wxid_xxxxxxxx"}))
}
func TestSendIMG(t *testing.T) {
	wcf, err := NewWCF("")
	if err != nil {
		logs.Err(err)
		return
	}
	logs.Info(wcf.SendIMG("C:\\Users\\danbai\\Desktop\\work\\code\\WeChatFerry-go\\图片1.png", "wxid_qvo0irhbw9fk22"))
}
func TestOnMSG(t *testing.T) {
	wcf, err := NewWCF("")
	if err != nil {
		logs.Err(err)
		return
	}
	logs.Info(wcf.EnableRecvTxt())
	defer wcf.DisableRecvTxt()
	wcf.OnMSG(func(msg *WxMsg) {
		logs.Info(msg.GetContent())
	})
}
