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
	add     string
	socket  protocol.Socket
	RecvTxt bool
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

/*AcceptFriend 接收好友请求
 * 接收好友请求
 *
 * @param v3 xml.attrib["encryptusername"]
 * @param v4 xml.attrib["ticket"]
 */
func (c *Client) AcceptFriend(v3, v4 string) int32 {
	req := genFunReq(Functions_FUNC_ACCEPT_FRIEND)
	q := Request_V{
		V: &Verification{
			V3: v3,
			V4: v4,
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
		M: &AddMembers{Roomid: roomID, Wxids: wxIDs},
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
func (c *Client) ReceiveTransfer(transferId, wxID string) int32 {
	req := genFunReq(Functions_FUNC_RECV_TRANSFER)
	q := Request_Tf{
		Tf: &Transfer{Tid: transferId, Wxid: wxID},
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
func (c *Client) DecryptImage(src, dst string) int32 {
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
	return recv.GetStatus()
}

func (c *Client) AddChatRoomMembers(roomId string, wxIds []string) int32 {
	req := genFunReq(Functions_FUNC_ADD_ROOM_MEMBERS)
	q := Request_M{
		M: &AddMembers{Roomid: roomId,
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

/*
SendTxt
@param msg:      消息内容（如果是 @ 消息则需要有跟 @ 的人数量相同的 @）
@param receiver: 消息接收人，私聊为 wxid（wxid_xxxxxxxxxxxxxx），群聊为roomid（xxxxxxxxxx@chatroom）
@param ates:    群聊时要 @ 的人（私聊时为空字符串），多个用逗号分隔。@所有人 用notify@all（必须是群主或者管理员才有权限）
*/
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

/*
SendIMG
path 绝对路径
*/
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

/*
SendFile
path 绝对路径
*/
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
func (c *Client) EnableRecvTxt() int32 {
	err := c.send(genFunReq(Functions_FUNC_ENABLE_RECV_TXT).build())
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
func (c *Client) OnMSG(f func(msg *WxMsg)) error {
	socket, err := pair1.NewSocket()
	if err != nil {
		return err
	}
	_ = socket.SetOption(mangos.OptionRecvDeadline, 2000)
	_ = socket.SetOption(mangos.OptionSendDeadline, 2000)
	err = socket.Dial(addPort(c.add))

	if err != nil {
		return err
	}
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
