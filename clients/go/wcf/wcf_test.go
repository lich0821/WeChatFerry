package wcf

import (
	logs "github.com/danbai225/go-logs"
	"go.nanomsg.org/mangos/v3/protocol/pair1"
	_ "go.nanomsg.org/mangos/v3/transport/all"
	"google.golang.org/protobuf/proto"
	"testing"
	"time"
)

func TestConn(t *testing.T) {
	socket, err := pair1.NewSocket()
	if err != nil {
		logs.Err(err)
		return
	}
	err = socket.Dial("tcp://192.168.26.130:1000")
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
	wcf, err := NewWCF("tcp://192.168.26.130:1000")
	if err != nil {
		logs.Err(err)
		return
	}
	logs.Info(wcf.IsLogin())
}
func TestGetSelfWXID(t *testing.T) {
	wcf, err := NewWCF("tcp://192.168.26.130:1000")
	if err != nil {
		logs.Err(err)
		return
	}
	logs.Info(wcf.GetSelfWXID())
}
func TestGetMsgTypes(t *testing.T) {
	wcf, err := NewWCF("tcp://192.168.26.130:1000")
	if err != nil {
		logs.Err(err)
		return
	}
	logs.Info(wcf.GetMsgTypes())
}
func TestGetContacts(t *testing.T) {
	wcf, err := NewWCF("tcp://192.168.26.130:1000")
	if err != nil {
		logs.Err(err)
		return
	}
	for _, contact := range wcf.GetContacts() {
		logs.Info(contact.Name, contact.Wxid)
	}
	logs.Info(wcf.GetContacts())
}
func TestGetDBNames(t *testing.T) {
	wcf, err := NewWCF("tcp://192.168.26.130:1000")
	if err != nil {
		logs.Err(err)
		return
	}
	logs.Info(wcf.GetDBNames())
}
func TestGetDBTables(t *testing.T) {
	wcf, err := NewWCF("tcp://192.168.26.130:1000")
	if err != nil {
		logs.Err(err)
		return
	}
	logs.Info(wcf.GetDBTables("ChatMsg.db"))
}
func TestExecDBQuery(t *testing.T) {
	wcf, err := NewWCF("tcp://192.168.26.130:1000")
	if err != nil {
		logs.Err(err)
		return
	}
	logs.Info(wcf.ExecDBQuery("ChatMsg.db", "SELECT * FROM Name2ID_v1"))
}
func TestAcceptFriend(t *testing.T) {
	wcf, err := NewWCF("tcp://192.168.26.130:1000")
	if err != nil {
		logs.Err(err)
		return
	}
	logs.Info(wcf.AcceptFriend("encryptusername", "ticket", 17))
}
func TestGetUserInfo(t *testing.T) {
	wcf, err := NewWCF("tcp://192.168.26.130:1000")
	if err != nil {
		logs.Err(err)
		return
	}
	logs.Info(wcf.GetUserInfo())
}
func TestSendTxT(t *testing.T) {
	wcf, err := NewWCF("tcp://192.168.26.130:1000")
	if err != nil {
		logs.Err(err)
		return
	}
	logs.Info(wcf.SendTxt(" Hello @ 淡白", "38975652309@chatroom", []string{"wxid_xxxxxx"}))
}
func TestSendIMG(t *testing.T) {
	wcf, err := NewWCF("tcp://192.168.26.130:1000")
	if err != nil {
		logs.Err(err)
		return
	}
	logs.Info(wcf.SendIMG("C:\\Users\\Administrator\\Pictures\\1.png", "wxid_xxxxxx"))
}
func TestSendFile(t *testing.T) {
	wcf, err := NewWCF("tcp://192.168.26.130:1000")
	if err != nil {
		logs.Err(err)
		return
	}
	logs.Info(wcf.SendFile("C:\\Users\\Administrator\\Pictures\\1.txt", "wxid_xxxxxx"))
}
func TestRefreshPYQ(t *testing.T) {
	wcf, err := NewWCF("tcp://192.168.26.130:1000")
	if err != nil {
		logs.Err(err)
		return
	}
	logs.Info(wcf.EnableRecvTxt())
	go wcf.OnMSG(func(msg *WxMsg) {
		logs.Info(msg.GetType(), msg.GetContent())
	})
	defer wcf.DisableRecvTxt()
	time.Sleep(time.Second * 2)
	logs.Info(wcf.RefreshPYQ())
	time.Sleep(time.Minute)
}

func TestOnMSG(t *testing.T) {
	wcf, err := NewWCF("tcp://192.168.26.130:1000")
	if err != nil {
		logs.Err(err)
		return
	}
	logs.Info(wcf.EnableRecvTxt())
	defer func() {
		logs.Info(wcf.DisableRecvTxt())
	}()
	go wcf.OnMSG(func(msg *WxMsg) {
		logs.Info(msg.GetContent())
	})
	time.Sleep(time.Minute)
}

func TestDisableRecvTxt(t *testing.T) {
	wcf, err := NewWCF("tcp://192.168.26.130:1000")
	if err != nil {
		logs.Err(err)
		return
	}
	logs.Info(wcf.DisableRecvTxt())
}
