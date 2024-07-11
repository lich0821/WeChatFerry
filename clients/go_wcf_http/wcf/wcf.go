package wcf

import (
	logs "github.com/danbai225/go-logs"
	"go.nanomsg.org/mangos/v3"
	"go.nanomsg.org/mangos/v3/protocol"
	"go.nanomsg.org/mangos/v3/protocol/pair1"
	_ "go.nanomsg.org/mangos/v3/transport/all"
	"google.golang.org/protobuf/proto"
	"strconv"
	"strings"
)

type Client struct {
	add                string
	socket             protocol.Socket
	RecvTxt            bool
	ContactsMap        []map[string]string
	MessageCallbackUrl string
}

func (c *Client) conn() error {
	socket, err := pair1.NewSocket()
	if err != nil {
		return err
	}
	err = socket.Dial(c.add)
	if err != nil {
		return err
	}
	c.socket = socket
	return err
}

func (c *Client) send(data []byte) error {
	return c.socket.Send(data)
}

func (c *Client) Recv() (*Response, error) {
	msg := &Response{}
	recv, err := c.socket.Recv()
	if err != nil {
		return msg, err
	}
	err = proto.Unmarshal(recv, msg)
	return msg, err
}

// Close 退出
func (c *Client) Close() error {
	c.DisableRecvTxt()
	return c.socket.Close()
}

// IsLogin 查看是否登录
func (c *Client) IsLogin() bool {
	err := c.send(genFunReq(Functions_FUNC_IS_LOGIN).build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}
	if recv.GetStatus() == 1 {
		return true
	}
	return false
}

// GetSelfWXID 获取登录的id
func (c *Client) GetSelfWXID() string {
	err := c.send(genFunReq(Functions_FUNC_GET_SELF_WXID).build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}
	return recv.GetStr()
}

// GetMsgTypes 获取消息类型
func (c *Client) GetMsgTypes() map[int32]string {
	err := c.send(genFunReq(Functions_FUNC_GET_MSG_TYPES).build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}
	return recv.GetTypes().GetTypes()
}

// GetContacts 获取通讯录
func (c *Client) GetContacts() []*RpcContact {
	err := c.send(genFunReq(Functions_FUNC_GET_CONTACTS).build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}
	return recv.GetContacts().GetContacts()
}

// GetDBNames 获取数据库名
func (c *Client) GetDBNames() []string {
	err := c.send(genFunReq(Functions_FUNC_GET_DB_NAMES).build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}
	return recv.GetDbs().Names
}

// GetDBTables 获取表
func (c *Client) GetDBTables(tab string) []*DbTable {
	req := genFunReq(Functions_FUNC_GET_DB_TABLES)
	str := &Request_Str{Str: tab}
	req.Msg = str
	err := c.send(req.build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}
	return recv.GetTables().GetTables()
}

// ExecDBQuery 执行sql
func (c *Client) ExecDBQuery(db, sql string) []*DbRow {
	req := genFunReq(Functions_FUNC_EXEC_DB_QUERY)
	q := Request_Query{
		Query: &DbQuery{
			Db:  db,
			Sql: sql,
		},
	}
	req.Msg = &q
	err := c.send(req.build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}
	return recv.GetRows().GetRows()
}

// AcceptFriend 接收好友请求
func (c *Client) AcceptFriend(v3, v4 string, scene int32) int32 {
	req := genFunReq(Functions_FUNC_ACCEPT_FRIEND)
	q := Request_V{
		V: &Verification{
			V3:    v3,
			V4:    v4,
			Scene: scene,
		}}

	req.Msg = &q
	err := c.send(req.build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}
	return recv.GetStatus()
}

func (c *Client) AddChatroomMembers(roomID, wxIDs string) int32 {
	req := genFunReq(Functions_FUNC_ADD_ROOM_MEMBERS)
	q := Request_M{
		M: &MemberMgmt{Roomid: roomID, Wxids: wxIDs},
	}
	req.Msg = &q
	err := c.send(req.build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}
	return recv.GetStatus()
}

// ReceiveTransfer 接收转账
func (c *Client) ReceiveTransfer(wxid, tfid, taid string) int32 {
	req := genFunReq(Functions_FUNC_RECV_TRANSFER)
	q := Request_Tf{
		Tf: &Transfer{
			Wxid: wxid,
			Tfid: tfid,
			Taid: taid,
		},
	}
	req.Msg = &q
	err := c.send(req.build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}
	return recv.GetStatus()
}

// RefreshPYQ 刷新朋友圈
func (c *Client) RefreshPYQ() int32 {
	req := genFunReq(Functions_FUNC_REFRESH_PYQ)
	q := Request_Ui64{
		Ui64: 0,
	}
	req.Msg = &q
	err := c.send(req.build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}
	return recv.GetStatus()
}

// DecryptImage 解密图片 加密路径，解密路径
func (c *Client) DecryptImage(src, dst string) string {
	req := genFunReq(Functions_FUNC_DECRYPT_IMAGE)
	q := Request_Dec{
		Dec: &DecPath{Src: src, Dst: dst},
	}
	req.Msg = &q
	err := c.send(req.build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}

	return recv.String()
}

// AddChatRoomMembers 添加群成员
func (c *Client) AddChatRoomMembers(roomId string, wxIds []string) int32 {
	req := genFunReq(Functions_FUNC_ADD_ROOM_MEMBERS)
	q := Request_M{
		M: &MemberMgmt{Roomid: roomId,
			Wxids: strings.Join(wxIds, ",")},
	}
	req.Msg = &q
	err := c.send(req.build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}
	return recv.GetStatus()
}

// InvChatRoomMembers 邀请群成员
func (c *Client) InvChatRoomMembers(roomId string, wxIds []string) int32 {
	req := genFunReq(Functions_FUNC_INV_ROOM_MEMBERS)
	q := Request_M{
		M: &MemberMgmt{Roomid: roomId,
			Wxids: strings.Join(wxIds, ",")},
	}
	req.Msg = &q
	err := c.send(req.build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}
	return recv.GetStatus()
}

// DelChatRoomMembers 删除群成员
func (c *Client) DelChatRoomMembers(roomId string, wxIds []string) int32 {
	req := genFunReq(Functions_FUNC_DEL_ROOM_MEMBERS)
	q := Request_M{
		M: &MemberMgmt{Roomid: roomId,
			Wxids: strings.Join(wxIds, ",")},
	}
	req.Msg = &q
	err := c.send(req.build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}
	return recv.GetStatus()
}

// GetUserInfo 获取自己的信息
func (c *Client) GetUserInfo() *UserInfo {
	err := c.send(genFunReq(Functions_FUNC_GET_USER_INFO).build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}
	return recv.GetUi()
}

// SendTxt 发送文本内容
func (c *Client) SendTxt(msg string, receiver string, ates []string) int32 {
	req := genFunReq(Functions_FUNC_SEND_TXT)
	req.Msg = &Request_Txt{
		Txt: &TextMsg{
			Msg:      msg,
			Receiver: receiver,
			Aters:    strings.Join(ates, ","),
		},
	}
	err := c.send(req.build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}
	return recv.GetStatus()
}

// ForwardMsg 转发消息
func (c *Client) ForwardMsg(Id uint64, receiver string) int32 {
	req := genFunReq(Functions_FUNC_FORWARD_MSG)
	req.Msg = &Request_Fm{
		Fm: &ForwardMsg{
			Id:       Id,
			Receiver: receiver,
		},
	}
	err := c.send(req.build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}
	return recv.GetStatus()
}

// SendIMG 发送图片
func (c *Client) SendIMG(path string, receiver string) int32 {
	req := genFunReq(Functions_FUNC_SEND_IMG)
	req.Msg = &Request_File{
		File: &PathMsg{
			Path:     path,
			Receiver: receiver,
		},
	}
	err := c.send(req.build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}
	return recv.GetStatus()
}

// SendFile 发送文件
func (c *Client) SendFile(path string, receiver string) int32 {
	req := genFunReq(Functions_FUNC_SEND_FILE)
	req.Msg = &Request_File{
		File: &PathMsg{
			Path:     path,
			Receiver: receiver,
		},
	}
	err := c.send(req.build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}
	return recv.GetStatus()
}

// SendRichText 发送卡片消息
func (c *Client) SendRichText(name string, account string, title string, digest string, url string, thumburl string, receiver string) int32 {
	req := genFunReq(Functions_FUNC_SEND_RICH_TXT)
	req.Msg = &Request_Rt{
		Rt: &RichText{
			Name:     name,
			Account:  account,
			Title:    title,
			Digest:   digest,
			Url:      url,
			Thumburl: thumburl,
			Receiver: receiver,
		},
	}
	err := c.send(req.build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}
	return recv.GetStatus()
}

// SendXml 发送xml数据
func (c *Client) SendXml(path, content, receiver string, Type int32) int32 {
	req := genFunReq(Functions_FUNC_SEND_XML)
	req.Msg = &Request_Xml{
		Xml: &XmlMsg{
			Receiver: receiver,
			Content:  content,
			Path:     path,
			Type:     Type,
		},
	}
	err := c.send(req.build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}
	return recv.GetStatus()
}

// SendEmotion 发送emoji
func (c *Client) SendEmotion(path, receiver string) int32 {
	req := genFunReq(Functions_FUNC_SEND_EMOTION)
	req.Msg = &Request_File{
		File: &PathMsg{
			Path:     path,
			Receiver: receiver,
		},
	}
	err := c.send(req.build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}
	return recv.GetStatus()
}

// SendPat 发送拍一拍消息
func (c *Client) SendPat(roomId, wxId string) int32 {
	req := genFunReq(Functions_FUNC_SEND_PAT_MSG)
	req.Msg = &Request_Pm{
		Pm: &PatMsg{
			Roomid: roomId,
			Wxid:   wxId,
		},
	}
	err := c.send(req.build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}
	return recv.GetStatus()
}

// DownloadAttach 下载附件
func (c *Client) DownloadAttach(id uint64, thumb, extra string) int32 {
	req := genFunReq(Functions_FUNC_SEND_PAT_MSG)
	req.Msg = &Request_Att{
		Att: &AttachMsg{
			Id:    id,
			Thumb: thumb,
			Extra: extra,
		},
	}
	err := c.send(req.build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}
	return recv.GetStatus()
}

// EnableRecvTxt 开启接收数据
func (c *Client) EnableRecvTxt() int32 {
	req := genFunReq(Functions_FUNC_ENABLE_RECV_TXT)
	req.Msg = &Request_Flag{
		Flag: true,
	}
	err := c.send(req.build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}
	c.RecvTxt = true
	return recv.GetStatus()
}

// DisableRecvTxt 关闭接收消息
func (c *Client) DisableRecvTxt() int32 {
	err := c.send(genFunReq(Functions_FUNC_DISABLE_RECV_TXT).build())
	if err != nil {
		logs.Err(err)
	}
	recv, err := c.Recv()
	if err != nil {
		logs.Err(err)
	}
	c.RecvTxt = false
	return recv.GetStatus()
}

// OnMSG 接收消息
func (c *Client) OnMSG(f func(msg *WxMsg)) error {
	socket, err := pair1.NewSocket()
	if err != nil {
		return err
	}
	_ = socket.SetOption(mangos.OptionRecvDeadline, 5000)
	_ = socket.SetOption(mangos.OptionSendDeadline, 5000)
	err = socket.Dial(addPort(c.add))
	if err != nil {
		return err
	}
	defer socket.Close()
	for c.RecvTxt {
		msg := &Response{}
		recv, err := socket.Recv()
		if err != nil {
			return err
		}
		_ = proto.Unmarshal(recv, msg)
		go f(msg.GetWxmsg())

	}
	return err
}

// NewWCF 连接
func NewWCF(add string) (*Client, error) {
	if add == "" {
		add = "tcp://127.0.0.1:10086"
	}
	client := &Client{add: add}
	err := client.conn()
	return client, err
}

type cmdMSG struct {
	*Request
}

func (c *cmdMSG) build() []byte {
	marshal, _ := proto.Marshal(c)
	return marshal
}

func genFunReq(fun Functions) *cmdMSG {
	return &cmdMSG{
		&Request{Func: fun,
			Msg: nil},
	}
}

func addPort(add string) string {
	parts := strings.Split(add, ":")
	port, _ := strconv.Atoi(parts[2])
	newPort := port + 1
	return parts[0] + ":" + parts[1] + ":" + strconv.Itoa(newPort)
}
