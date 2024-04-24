use std::{env, path::PathBuf, process::Command, time::Duration, vec};

use log::{error, info, warn};
use nng::options::{Options, RecvTimeout};
use prost::Message;
use std::collections::HashMap;

const DEFAULT_URL: &'static str = "tcp://127.0.0.1:10086";
const LISTEN_URL: &'static str = "tcp://127.0.0.1:10087";

pub mod wcf {
    include!("proto/wcf.rs");
}

pub mod roomdata {
    include!("proto/roomdata.rs");
}

#[derive(Clone, Debug)]
pub struct WeChat {
    pub url: String,
    pub wcf_path: PathBuf,
    pub debug: bool,
    pub socket: nng::Socket,
    pub listening: bool,
    pub enable_accept_firend: bool,
}

#[derive(Clone, Debug)]
pub struct UserInfo {
    pub wxid: String,
    pub name: String,
    pub mobile: String,
    pub home: String,
}

#[derive(Clone, Debug, Default)]
pub struct ChatRoom {
    /// 群聊ID
    pub room_id: String,
    /// 群聊成员
    pub room_data: roomdata::RoomData,
    /// 群聊头像
    pub room_head_img_url: Option<String>,
    /// 公告
    pub room_announcement: Option<String>,
}

#[derive(Clone, Debug, Default)]
pub struct ContactInfo {
    /// 微信ID
    pub wxid: String,
    /// 微信号
    pub alias: Option<String>,
    /// 删除标记
    pub del_flag: u8,
    /// 类型
    pub contact_type: u8,
    /// 备注
    pub remark: Option<String>,
    /// 昵称
    pub nick_name: Option<String>,
    /// 昵称拼音首字符
    pub py_initial: Option<String>,
    /// 昵称全拼
    pub quan_pin: Option<String>,
    /// 备注拼音首字母
    pub remark_py_initial: Option<String>,
    /// 备注全拼
    pub remark_quan_pin: Option<String>,
    /// 小头像
    pub small_head_url: Option<String>,
    /// 大头像
    pub big_head_url: Option<String>,
}

impl Default for WeChat {
    fn default() -> Self {
        WeChat::new(false)
    }
}

impl WeChat {
    pub fn new(debug: bool) -> Self {
        let path = env::current_dir().unwrap().join("lib").join("wcf.exe");
        let _ = start(path.clone(), debug);
        let socket = connect(&DEFAULT_URL).unwrap();
        WeChat {
            url: String::from(DEFAULT_URL),
            wcf_path: path,
            debug,
            socket,
            listening: false,
            enable_accept_firend: false,
        }
    }
}

fn start(wcf_path: PathBuf, debug: bool) -> Result<(), Box<dyn std::error::Error>> {
    let mut args = vec!["start", "10086"];
    if debug {
        args.push("debug");
    }
    info!(
        "wcf_path: {}, debug: {}",
        wcf_path.clone().to_str().unwrap(),
        debug
    );
    let _ = match Command::new(wcf_path.to_str().unwrap()).args(args).output() {
        Ok(output) => output,
        Err(e) => {
            error!("命令行启动失败: {}", e);
            return Err("服务启动失败".into());
        }
    };
    Ok(())
}

pub fn stop(wechat: &mut WeChat) -> Result<(), Box<dyn std::error::Error>> {
    let _ = disable_listen(wechat);
    wechat.socket.close();
    let output = Command::new(wechat.wcf_path.to_str().unwrap())
        .args(["stop"])
        .output();
    let _output = match output {
        Ok(output) => output,
        Err(e) => {
            error!("服务停止失败: {}", e);
            return Err("服务停止失败".into());
        }
    };
    info!("服务已停止: {}", wechat.url);
    Ok(())
}

fn connect(url: &str) -> Result<nng::Socket, Box<dyn std::error::Error>> {
    let client = match nng::Socket::new(nng::Protocol::Pair1) {
        Ok(client) => client,
        Err(e) => {
            error!("Socket创建失败: {}", e);
            return Err("连接服务失败".into());
        }
    };
    match client.set_opt::<RecvTimeout>(Some(Duration::from_millis(5000))) {
        Ok(()) => (),
        Err(e) => {
            error!("连接参数设置失败: {}", e);
            return Err("连接参数设置失败".into());
        }
    };
    match client.set_opt::<nng::options::SendTimeout>(Some(Duration::from_millis(5000))) {
        Ok(()) => (),
        Err(e) => {
            error!("连接参数设置失败: {}", e);
            return Err("连接参数设置失败".into());
        }
    };
    match client.dial(url) {
        Ok(()) => (),
        Err(e) => {
            error!("连接服务失败: {}", e);
            return Err("连接服务失败".into());
        }
    };
    Ok(client)
}

fn send_cmd(
    wechat: &WeChat,
    req: wcf::Request,
) -> Result<Option<wcf::response::Msg>, Box<dyn std::error::Error>> {
    let mut buf = Vec::with_capacity(req.encoded_len());
    match req.encode(&mut buf) {
        Ok(()) => (),
        Err(e) => {
            error!("序列化失败: {}", e);
            return Err("通信失败".into());
        }
    };
    let msg = nng::Message::from(&buf[..]);
    let _ = match wechat.socket.send(msg) {
        Ok(()) => {}
        Err(e) => {
            error!("Socket发送失败: {:?}, {}", e.0, e.1);
            return Err("通信失败".into());
        }
    };
    let mut msg = match wechat.socket.recv() {
        Ok(msg) => msg,
        Err(e) => {
            error!("Socket接收失败: {}", e);
            return Err("通信失败".into());
        }
    };
    // 反序列化为prost消息
    let response = match wcf::Response::decode(msg.as_slice()) {
        Ok(res) => res,
        Err(e) => {
            error!("反序列化失败: {}", e);
            return Err("通信失败".into());
        }
    };
    msg.clear();
    Ok(response.msg)
}

pub fn is_login(wechat: &WeChat) -> Result<bool, Box<dyn std::error::Error>> {
    let req = wcf::Request {
        func: wcf::Functions::FuncIsLogin.into(),
        msg: None,
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("登录状态检查失败".into());
        }
    };
    if response.is_none() {
        return Ok(false);
    }
    match response.unwrap() {
        wcf::response::Msg::Status(status) => {
            return Ok(1 == status);
        }
        _ => {
            return Ok(false);
        }
    };
}

pub fn get_self_wx_id(wechat: &mut WeChat) -> Result<Option<String>, Box<dyn std::error::Error>> {
    let req = wcf::Request {
        func: wcf::Functions::FuncGetSelfWxid.into(),
        msg: None,
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("获取微信ID失败".into());
        }
    };
    if response.is_none() {
        return Ok(None);
    }
    match response.unwrap() {
        wcf::response::Msg::Str(wx_id) => {
            return Ok(Some(wx_id));
        }
        _ => {
            return Ok(None);
        }
    };
}

pub fn get_user_info(wechat: &mut WeChat) -> Result<Option<UserInfo>, Box<dyn std::error::Error>> {
    let req = wcf::Request {
        func: wcf::Functions::FuncGetUserInfo.into(),
        msg: None,
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("获取用户信息失败".into());
        }
    };
    if response.is_none() {
        return Ok(None);
    }
    match response.unwrap() {
        wcf::response::Msg::Ui(user_info) => {
            return Ok(Some(UserInfo {
                wxid: user_info.wxid,
                name: user_info.name,
                mobile: user_info.mobile,
                home: user_info.home,
            }));
        }
        _ => {
            return Ok(None);
        }
    };
}

pub fn get_contacts(
    wechat: &mut WeChat,
) -> Result<Option<wcf::RpcContacts>, Box<dyn std::error::Error>> {
    info!("获取联系人");
    let req = wcf::Request {
        func: wcf::Functions::FuncGetContacts.into(),
        msg: None,
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("获取微信联系人失败".into());
        }
    };
    if response.is_none() {
        return Ok(None);
    }
    match response.unwrap() {
        wcf::response::Msg::Contacts(contact) => {
            return Ok(Some(contact));
        }
        _ => {
            return Ok(None);
        }
    };
}

pub fn query_all_contact_info(
    wechat: &mut WeChat,
) -> Result<Vec<ContactInfo>, Box<dyn std::error::Error>> {
    let contacts = crate::wechat::exec_db_query(
        wechat,
        String::from("MicroMsg.db"),
        String::from("select * from Contact"),
    )
    .unwrap();
    if contacts.len() == 0 {
        return Ok(vec![]);
    } else {
        let mut contact_list: Vec<ContactInfo> = vec![];
        for contact in contacts.into_iter() {
            let mut contact_info = ContactInfo::default();
            let fields = &contact.fields;
            for field in fields.into_iter() {
                if field.column.eq("UserName") {
                    contact_info.wxid = String::from_utf8(field.content.clone()).unwrap();
                } else if field.column.eq("Alias") {
                    contact_info.alias = Some(String::from_utf8(field.content.clone()).unwrap());
                } else if field.column.eq("DelFlag") {
                    contact_info.del_flag = *field.content.get(0).unwrap();
                } else if field.column.eq("Type") {
                    contact_info.contact_type = *field.content.get(0).unwrap();
                } else if field.column.eq("Remark") {
                    contact_info.remark = Some(String::from_utf8(field.content.clone()).unwrap());
                } else if field.column.eq("NickName") {
                    contact_info.nick_name =
                        Some(String::from_utf8(field.content.clone()).unwrap());
                } else if field.column.eq("PYInitial") {
                    contact_info.py_initial =
                        Some(String::from_utf8(field.content.clone()).unwrap());
                } else if field.column.eq("QuanPin") {
                    contact_info.quan_pin = Some(String::from_utf8(field.content.clone()).unwrap());
                } else if field.column.eq("RemarkPYInitial") {
                    contact_info.remark_py_initial =
                        Some(String::from_utf8(field.content.clone()).unwrap());
                } else if field.column.eq("RemarkQuanPin") {
                    contact_info.remark_quan_pin =
                        Some(String::from_utf8(field.content.clone()).unwrap());
                }
            }
            if !contact_info.wxid.is_empty() {
                let contact_heads = crate::wechat::exec_db_query(
                    wechat,
                    String::from("MicroMsg.db"),
                    String::from(format!(
                        "select * from ContactHeadImgUrl where usrName = '{}'",
                        contact_info.wxid
                    )),
                )
                .unwrap();
                if contact_heads.len() > 0 {
                    let contact_head = contact_heads.get(0).unwrap();
                    let head_fields = &contact_head.fields;
                    for field in head_fields.into_iter() {
                        if field.column.eq("smallHeadImgUrl") {
                            contact_info.small_head_url =
                                Some(String::from_utf8(field.content.clone()).unwrap());
                        } else if field.column.eq("bigHeadImgUrl") {
                            contact_info.big_head_url =
                                Some(String::from_utf8(field.content.clone()).unwrap());
                        }
                    }
                }
            }
            contact_list.push(contact_info);
        }

        Ok(contact_list)
    }
}

pub fn query_contact_info(
    wechat: &mut WeChat,
    wxid: String,
) -> Result<Option<ContactInfo>, Box<dyn std::error::Error>> {
    let contacts = crate::wechat::exec_db_query(
        wechat,
        String::from("MicroMsg.db"),
        String::from(format!(
            "select * from Contact where UserName = '{}'",
            wxid.clone()
        )),
    )
    .unwrap();
    if contacts.len() == 0 {
        return Ok(None);
    } else {
        let contact = contacts.get(0).unwrap();
        let mut contact_info = ContactInfo::default();
        let fields = &contact.fields;
        for field in fields.into_iter() {
            if field.column.eq("UserName") {
                contact_info.wxid = String::from_utf8(field.content.clone()).unwrap();
            } else if field.column.eq("Alias") {
                contact_info.alias = Some(String::from_utf8(field.content.clone()).unwrap());
            } else if field.column.eq("DelFlag") {
                contact_info.del_flag = *field.content.get(0).unwrap();
            } else if field.column.eq("Type") {
                contact_info.contact_type = *field.content.get(0).unwrap();
            } else if field.column.eq("Remark") {
                contact_info.remark = Some(String::from_utf8(field.content.clone()).unwrap());
            } else if field.column.eq("NickName") {
                contact_info.nick_name = Some(String::from_utf8(field.content.clone()).unwrap());
            } else if field.column.eq("PYInitial") {
                contact_info.py_initial = Some(String::from_utf8(field.content.clone()).unwrap());
            } else if field.column.eq("QuanPin") {
                contact_info.quan_pin = Some(String::from_utf8(field.content.clone()).unwrap());
            } else if field.column.eq("RemarkPYInitial") {
                contact_info.remark_py_initial =
                    Some(String::from_utf8(field.content.clone()).unwrap());
            } else if field.column.eq("RemarkQuanPin") {
                contact_info.remark_quan_pin =
                    Some(String::from_utf8(field.content.clone()).unwrap());
            }
        }
        let contact_heads = crate::wechat::exec_db_query(
            wechat,
            String::from("MicroMsg.db"),
            String::from(format!(
                "select * from ContactHeadImgUrl where usrName = '{}'",
                wxid
            )),
        )
        .unwrap();
        if contact_heads.len() > 0 {
            let contact_head = contact_heads.get(0).unwrap();
            let head_fields = &contact_head.fields;
            for field in head_fields.into_iter() {
                if field.column.eq("smallHeadImgUrl") {
                    contact_info.small_head_url =
                        Some(String::from_utf8(field.content.clone()).unwrap());
                } else if field.column.eq("bigHeadImgUrl") {
                    contact_info.big_head_url =
                        Some(String::from_utf8(field.content.clone()).unwrap());
                }
            }
        }
        Ok(Some(contact_info))
    }
}

pub fn query_chat_room_info(
    wechat: &mut WeChat,
    wxid: String,
) -> Result<Option<ChatRoom>, Box<dyn std::error::Error>> {
    let contacts = crate::wechat::exec_db_query(
        wechat,
        String::from("MicroMsg.db"),
        String::from(format!(
            "select * from ChatRoom where ChatRoomName = '{}'",
            wxid.clone()
        )),
    )
    .unwrap();
    if contacts.len() == 0 {
        return Ok(None);
    } else {
        let contact = contacts.get(0).unwrap();
        let mut chat_room = ChatRoom::default();
        let fields = &contact.fields;
        for field in fields.into_iter() {
            if field.column.eq("ChatRoomName") {
                chat_room.room_id = String::from_utf8(field.content.clone()).unwrap();
            } else if field.column.eq("RoomData") {
                chat_room.room_data = roomdata::RoomData::decode(field.content.as_slice()).unwrap();
            }
        }
        let contact_heads = crate::wechat::exec_db_query(
            wechat,
            String::from("MicroMsg.db"),
            String::from(format!(
                "select * from ContactHeadImgUrl where usrName = '{}'",
                wxid
            )),
        )
        .unwrap();
        if contact_heads.len() > 0 {
            let contact_head = contact_heads.get(0).unwrap();
            let head_fields = &contact_head.fields;
            for field in head_fields.into_iter() {
                if field.column.eq("smallHeadImgUrl") {
                    chat_room.room_head_img_url =
                        Some(String::from_utf8(field.content.clone()).unwrap());
                }
            }
        }
        let contact_heads = crate::wechat::exec_db_query(
            wechat,
            String::from("MicroMsg.db"),
            String::from(format!(
                "select * from ChatRoomInfo where ChatRoomName = '{}'",
                wxid
            )),
        )
        .unwrap();
        if contact_heads.len() > 0 {
            let contact_head = contact_heads.get(0).unwrap();
            let head_fields = &contact_head.fields;
            for field in head_fields.into_iter() {
                if field.column.eq("Announcement") {
                    chat_room.room_announcement =
                        Some(String::from_utf8(field.content.clone()).unwrap());
                }
            }
        }
        Ok(Some(chat_room))
    }
}

pub fn get_db_names(wechat: &mut WeChat) -> Result<Vec<String>, Box<dyn std::error::Error>> {
    let req = wcf::Request {
        func: wcf::Functions::FuncGetDbNames.into(),
        msg: None,
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("获取微信数据库失败".into());
        }
    };
    if response.is_none() {
        return Ok(vec![]);
    }
    match response.unwrap() {
        wcf::response::Msg::Dbs(dbs) => {
            return Ok(dbs.names);
        }
        _ => {
            return Ok(vec![]);
        }
    };
}

pub fn get_db_tables(
    wechat: &mut WeChat,
    db: String,
) -> Result<Vec<wcf::DbTable>, Box<dyn std::error::Error>> {
    let req = wcf::Request {
        func: wcf::Functions::FuncGetDbTables.into(),
        msg: Some(wcf::request::Msg::Str(db)),
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("获取数据库表失败".into());
        }
    };
    if response.is_none() {
        return Ok(vec![]);
    }
    match response.unwrap() {
        wcf::response::Msg::Tables(tables) => {
            let tables = tables.tables;
            return Ok(tables);
        }
        _ => {
            return Ok(vec![]);
        }
    };
}

pub fn exec_db_query(
    wechat: &mut WeChat,
    db: String,
    sql: String,
) -> Result<Vec<wcf::DbRow>, Box<dyn std::error::Error>> {
    let req = wcf::Request {
        func: wcf::Functions::FuncExecDbQuery.into(),
        msg: Some(wcf::request::Msg::Query(wcf::DbQuery { db, sql })),
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("执行SQL失败".into());
        }
    };
    if response.is_none() {
        return Ok(vec![]);
    }
    match response.unwrap() {
        wcf::response::Msg::Rows(rows) => {
            let rows = rows.rows;
            return Ok(rows);
        }
        _ => {
            return Ok(vec![]);
        }
    };
}

/**
 * @param msg:      消息内容（如果是 @ 消息则需要有跟 @ 的人数量相同的 @）
 * @param receiver: 消息接收人，私聊为 wxid（wxid_xxxxxxxxxxxxxx），群聊为
 *                  roomid（xxxxxxxxxx@chatroom）
 * @param aters:    群聊时要 @ 的人（私聊时为空字符串），多个用逗号分隔。@所有人 用
 *                  notify@all（必须是群主或者管理员才有权限）
 * @return int
 * @Description 发送文本消息
 * @author Changhua
 * @example sendText(" Hello @ 某人1 @ 某人2 ", " xxxxxxxx @ chatroom ",
 * "wxid_xxxxxxxxxxxxx1,wxid_xxxxxxxxxxxxx2");
 */
pub fn send_text(
    wechat: &mut WeChat,
    msg: String,
    receiver: String,
    aters: String,
) -> Result<bool, Box<dyn std::error::Error>> {
    let text_msg = wcf::TextMsg {
        msg,
        receiver,
        aters,
    };
    let req = wcf::Request {
        func: wcf::Functions::FuncSendTxt.into(),
        msg: Some(wcf::request::Msg::Txt(text_msg)),
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("微信消息发送失败".into());
        }
    };
    if response.is_none() {
        return Ok(false);
    }
    return Ok(true);
    // match response.unwrap() {
    //     wcf::response::Msg::Status(status) => {
    //         return Ok(1 == status);
    //     }
    //     _ => {
    //         return Ok(false);
    //     }
    // };
}

pub fn send_image(
    wechat: &mut WeChat,
    path: PathBuf,
    receiver: String,
) -> Result<bool, Box<dyn std::error::Error>> {
    let image_msg = wcf::PathMsg {
        path: String::from(path.to_str().unwrap()),
        receiver,
    };
    let req = wcf::Request {
        func: wcf::Functions::FuncSendImg.into(),
        msg: Some(wcf::request::Msg::File(image_msg)),
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("图片发送失败".into());
        }
    };
    println!("{:?}", response);
    if response.is_none() {
        return Ok(false);
    }
    return Ok(true);
    // match response.unwrap() {
    //     wcf::response::Msg::Status(status) => {
    //         return Ok(1 == status);
    //     }
    //     _ => {
    //         return Ok(false);
    //     }
    // };
}

pub fn send_file(
    wechat: &mut WeChat,
    path: PathBuf,
    receiver: String,
) -> Result<bool, Box<dyn std::error::Error>> {
    let image_msg = wcf::PathMsg {
        path: String::from(path.to_str().unwrap()),
        receiver,
    };
    let req = wcf::Request {
        func: wcf::Functions::FuncSendFile.into(),
        msg: Some(wcf::request::Msg::File(image_msg)),
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("文件发送失败".into());
        }
    };
    if response.is_none() {
        return Ok(false);
    }
    match response.unwrap() {
        wcf::response::Msg::Status(status) => {
            return Ok(1 == status);
        }
        _ => {
            return Ok(false);
        }
    };
}

pub fn send_xml(
    wechat: &mut WeChat,
    xml: String,
    path: PathBuf,
    receiver: String,
    xml_type: i32,
) -> Result<bool, Box<dyn std::error::Error>> {
    let xml_msg = wcf::XmlMsg {
        content: xml,
        path: String::from(path.to_str().unwrap()),
        receiver,
        r#type: xml_type,
    };
    let req = wcf::Request {
        func: wcf::Functions::FuncSendXml.into(),
        msg: Some(wcf::request::Msg::Xml(xml_msg)),
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("微信XML消息发送失败".into());
        }
    };
    if response.is_none() {
        return Ok(false);
    }
    match response.unwrap() {
        wcf::response::Msg::Status(status) => {
            return Ok(1 == status);
        }
        _ => {
            return Ok(false);
        }
    };
}

pub fn send_emotion(
    wechat: &mut WeChat,
    path: PathBuf,
    receiver: String,
) -> Result<bool, Box<dyn std::error::Error>> {
    let image_msg = wcf::PathMsg {
        path: String::from(path.to_str().unwrap()),
        receiver,
    };
    let req = wcf::Request {
        func: wcf::Functions::FuncSendEmotion.into(),
        msg: Some(wcf::request::Msg::File(image_msg)),
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("微信表情发送失败".into());
        }
    };
    if response.is_none() {
        return Ok(false);
    }
    match response.unwrap() {
        wcf::response::Msg::Status(status) => {
            return Ok(1 == status);
        }
        _ => {
            return Ok(false);
        }
    };
}

pub fn enable_listen(wechat: &mut WeChat) -> Result<nng::Socket, Box<dyn std::error::Error>> {
    if wechat.listening {
        return Err("消息接收服务已开启".into());
    }
    let req = wcf::Request {
        func: wcf::Functions::FuncEnableRecvTxt.into(),
        msg: Some(wcf::request::Msg::Flag(true)),
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("消息接收服务启动失败".into());
        }
    };
    if response.is_none() {
        return Err("消息接收服务启动失败".into());
    }
    let client = connect(LISTEN_URL).unwrap();
    wechat.listening = true;
    Ok(client)
}

pub fn disable_listen(wechat: &mut WeChat) -> Result<bool, Box<dyn std::error::Error>> {
    if !wechat.listening {
        return Ok(true);
    }
    let req = wcf::Request {
        func: wcf::Functions::FuncDisableRecvTxt.into(),
        msg: None,
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("消息接收服务停止失败".into());
        }
    };
    if response.is_none() {
        return Err("消息接收服务停止失败".into());
    }
    wechat.listening = false;
    return Ok(true);
}

pub fn recv_msg(client: &nng::Socket) -> Result<Option<wcf::WxMsg>, Box<dyn std::error::Error>> {
    let mut msg = match client.recv() {
        Ok(msg) => msg,
        Err(e) => {
            warn!("Socket消息接收失败: {}", e);
            return Ok(None);
        }
    };
    // 反序列化为prost消息
    let res = match wcf::Response::decode(msg.as_slice()) {
        Ok(res) => res,
        Err(e) => {
            error!("反序列化失败: {}", e);
            return Err("微信消息接收失败".into());
        }
    };
    msg.clear();
    let res_msg = res.msg;
    match res_msg {
        Some(wcf::response::Msg::Wxmsg(msg)) => {
            return Ok(Some(msg));
        }
        _ => {
            return Ok(None);
        }
    }
}

/**
 * 获取消息类型
 * {"47": "石头剪刀布 | 表情图片", "62": "小视频", "43": "视频", "1": "文字", "10002": "撤回消息", "40": "POSSIBLEFRIEND_MSG", "10000": "红包、系统消息", "37": "好友确认", "48": "位置", "42": "名片", "49": "共享实时位置、文件、转账、链接", "3": "图片", "34": "语音", "9999": "SYSNOTICE", "52": "VOIPNOTIFY", "53": "VOIPINVITE", "51": "微信初始化", "50": "VOIPMSG"}
 */
pub fn get_msg_types(
    wechat: &mut WeChat,
) -> Result<HashMap<i32, String>, Box<dyn std::error::Error>> {
    let req = wcf::Request {
        func: wcf::Functions::FuncGetMsgTypes.into(),
        msg: None,
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("获取消息类型失败".into());
        }
    };
    if response.is_none() {
        return Ok(HashMap::default());
    }
    match response.unwrap() {
        wcf::response::Msg::Types(msg_types) => {
            let types = msg_types.types;
            return Ok(types);
        }
        _ => {
            return Ok(HashMap::default());
        }
    };
}

pub fn accept_new_friend(
    v3: String,
    v4: String,
    scene: i32,
    wechat: &mut WeChat,
) -> Result<bool, Box<dyn std::error::Error>> {
    let req = wcf::Request {
        func: wcf::Functions::FuncAcceptFriend.into(),
        msg: Some(wcf::request::Msg::V(wcf::Verification { v3, v4, scene })),
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("同意加好友请求失败".into());
        }
    };
    if response.is_none() {
        return Ok(false);
    }
    match response.unwrap() {
        wcf::response::Msg::Status(status) => {
            return Ok(status == 1);
        }
        _ => {
            return Ok(false);
        }
    };
}

/* 添加群成员 */
pub fn add_chatroom_members(
    roomid: String,
    wxids: String,
    wechat: &mut WeChat,
) -> Result<bool, Box<dyn std::error::Error>> {
    let req = wcf::Request {
        func: wcf::Functions::FuncAddRoomMembers.into(),
        msg: Some(wcf::request::Msg::M(wcf::MemberMgmt { roomid, wxids })),
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("微信群加人失败".into());
        }
    };
    if response.is_none() {
        return Ok(false);
    }
    match response.unwrap() {
        wcf::response::Msg::Status(status) => {
            return Ok(status == 1);
        }
        _ => {
            return Ok(false);
        }
    };
}

/* 邀请群成员 */
pub fn inv_chatroom_members(
    roomid: String,
    wxids: String,
    wechat: &mut WeChat,
) -> Result<bool, Box<dyn std::error::Error>> {
    let req = wcf::Request {
        func: wcf::Functions::FuncInvRoomMembers.into(),
        msg: Some(wcf::request::Msg::M(wcf::MemberMgmt { roomid, wxids })),
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("邀请群成员失败".into());
        }
    };
    if response.is_none() {
        return Ok(false);
    }
    match response.unwrap() {
        wcf::response::Msg::Status(status) => {
            return Ok(status == 1);
        }
        _ => {
            return Ok(false);
        }
    };
}

/* 删除群成员 */
pub fn del_chatroom_members(
    roomid: String,
    wxids: String,
    wechat: &mut WeChat,
) -> Result<bool, Box<dyn std::error::Error>> {
    let req = wcf::Request {
        func: wcf::Functions::FuncDelRoomMembers.into(),
        msg: Some(wcf::request::Msg::M(wcf::MemberMgmt { roomid, wxids })),
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("微信群踢人失败".into());
        }
    };
    if response.is_none() {
        return Ok(false);
    }
    match response.unwrap() {
        wcf::response::Msg::Status(status) => {
            return Ok(status == 1);
        }
        _ => {
            return Ok(false);
        }
    };
}

pub fn decrypt_image(
    src: String,
    dst: String,
    wechat: &mut WeChat,
) -> Result<bool, Box<dyn std::error::Error>> {
    let req = wcf::Request {
        func: wcf::Functions::FuncDecryptImage.into(),
        msg: Some(wcf::request::Msg::Dec(wcf::DecPath { src, dst })),
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("图片解密失败".into());
        }
    };
    if response.is_none() {
        return Ok(false);
    }
    match response.unwrap() {
        wcf::response::Msg::Status(status) => {
            return Ok(status == 1);
        }
        _ => {
            return Ok(false);
        }
    };
}

pub fn recv_transfer(
    wxid: String,
    transferid: String,
    transcationid: String,
    wechat: &mut WeChat,
) -> Result<bool, Box<dyn std::error::Error>> {
    let req = wcf::Request {
        func: wcf::Functions::FuncRecvTransfer.into(),
        msg: Some(wcf::request::Msg::Tf(wcf::Transfer {
            wxid: wxid,
            tfid: transferid,
            taid: transcationid,
        })),
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("接收转账失败".into());
        }
    };
    if response.is_none() {
        return Ok(false);
    }
    match response.unwrap() {
        wcf::response::Msg::Status(status) => {
            return Ok(status == 1);
        }
        _ => {
            return Ok(false);
        }
    };
}

/** 刷新朋友圈 */
pub fn refresh_pyq(id: u64, wechat: &mut WeChat) -> Result<bool, Box<dyn std::error::Error>> {
    let req = wcf::Request {
        func: wcf::Functions::FuncRefreshPyq.into(),
        msg: Some(wcf::request::Msg::Ui64(id)),
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("接收转账失败".into());
        }
    };
    if response.is_none() {
        return Ok(false);
    }
    match response.unwrap() {
        wcf::response::Msg::Status(status) => {
            return Ok(status != -1);
        }
        _ => {
            return Ok(false);
        }
    };
}

/** 保存附件 */
pub fn attach_msg(
    id: u64,
    thumb: String,
    extra: String,
    wechat: &mut WeChat,
) -> Result<bool, Box<dyn std::error::Error>> {
    let req = wcf::Request {
        func: wcf::Functions::FuncDownloadAttach.into(),
        msg: Some(wcf::request::Msg::Att(wcf::AttachMsg { id, thumb, extra })),
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("保存附件失败".into());
        }
    };
    if response.is_none() {
        return Ok(false);
    }
    match response.unwrap() {
        wcf::response::Msg::Status(status) => {
            return Ok(status != -1);
        }
        _ => {
            return Ok(false);
        }
    };
}

/** 获取语音 */
pub fn get_audio_msg(
    id: u64,
    dir: String,
    wechat: &mut WeChat,
) -> Result<bool, Box<dyn std::error::Error>> {
    let req = wcf::Request {
        func: wcf::Functions::FuncGetAudioMsg.into(),
        msg: Some(wcf::request::Msg::Am(wcf::AudioMsg { id, dir })),
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("获取语音失败".into());
        }
    };
    if response.is_none() {
        return Ok(false);
    }
    match response.unwrap() {
        wcf::response::Msg::Status(status) => {
            return Ok(status != -1);
        }
        _ => {
            return Ok(false);
        }
    };
}

/** 发送富文本 */
pub fn sned_rich_text(
    name: String,
    account: String,
    title: String,
    digest: String,
    url: String,
    thumburl: String,
    receiver: String,
    wechat: &mut WeChat,
) -> Result<bool, Box<dyn std::error::Error>> {
    let req = wcf::Request {
        func: wcf::Functions::FuncSendRichTxt.into(),
        msg: Some(wcf::request::Msg::Rt(wcf::RichText {
            name,
            account,
            title,
            digest,
            url,
            thumburl,
            receiver,
        })),
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("发送富文本失败".into());
        }
    };
    if response.is_none() {
        return Ok(false);
    }
    match response.unwrap() {
        wcf::response::Msg::Status(status) => {
            return Ok(status != -1);
        }
        _ => {
            return Ok(false);
        }
    };
}

/** 发送拍一拍 */
pub fn sned_pat_msg(
    roomid: String,
    wxid: String,
    wechat: &mut WeChat,
) -> Result<bool, Box<dyn std::error::Error>> {
    let req = wcf::Request {
        func: wcf::Functions::FuncSendPatMsg.into(),
        msg: Some(wcf::request::Msg::Pm(wcf::PatMsg { roomid, wxid })),
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("发送拍一拍失败".into());
        }
    };
    if response.is_none() {
        return Ok(false);
    }
    match response.unwrap() {
        wcf::response::Msg::Status(status) => {
            return Ok(status != -1);
        }
        _ => {
            return Ok(false);
        }
    };
}

/** OCR */
pub fn exec_ocr(
    path: PathBuf,
    wechat: &mut WeChat,
) -> Result<Option<wcf::OcrMsg>, Box<dyn std::error::Error>> {
    let req = wcf::Request {
        func: wcf::Functions::FuncExecOcr.into(),
        msg: Some(wcf::request::Msg::Str(String::from(path.to_str().unwrap()))),
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("OCR失败".into());
        }
    };
    if response.is_none() {
        return Ok(None);
    }
    match response.unwrap() {
        wcf::response::Msg::Ocr(msg) => {
            return Ok(Some(msg));
        }
        _ => {
            return Ok(None);
        }
    };
}

/** 转发消息 */
pub fn forward_msg(
    id: u64,
    receiver: String,
    wechat: &mut WeChat,
) -> Result<bool, Box<dyn std::error::Error>> {
    let req = wcf::Request {
        func: wcf::Functions::FuncForwardMsg.into(),
        msg: Some(wcf::request::Msg::Fm(wcf::ForwardMsg { id, receiver })),
    };
    let response = match send_cmd(wechat, req) {
        Ok(res) => res,
        Err(e) => {
            error!("命令发送失败: {}", e);
            return Err("转发消息".into());
        }
    };
    if response.is_none() {
        return Ok(false);
    }
    match response.unwrap() {
        wcf::response::Msg::Status(status) => {
            return Ok(status != -1);
        }
        _ => {
            return Ok(false);
        }
    };
}

mod test {

    #[test]
    fn test_start_stop() {
        use std::thread;
        use std::time::Duration;

        let mut wechat = crate::wechat::WeChat::new(false);
        thread::sleep(Duration::from_secs(20));
        let _ = crate::wechat::stop(&mut wechat);
    }

    #[test]
    fn test_is_login() {
        let mut wechat = crate::wechat::WeChat::default();
        let is_login = crate::wechat::is_login(&mut wechat).unwrap();
        println!("IsLogin: {}", is_login);
    }

    #[test]
    fn test_get_db_names() {
        let mut wechat = crate::wechat::WeChat::default();
        let dbs = crate::wechat::get_db_names(&mut wechat).unwrap();
        println!("dbs: {:?}", dbs);
    }

    #[test]
    fn test_get_db_tables() {
        let mut wechat = crate::wechat::WeChat::default();
        let tables: Vec<crate::wechat::wcf::DbTable> =
            crate::wechat::get_db_tables(&mut wechat, String::from("MicroMsg.db")).unwrap();
        println!("tables: {:?}", tables);
    }

    #[test]
    fn test_exec_db_query_contact() {
        use hex;
        pub mod roomdata {
            include!("proto/roomdata.rs");
        }

        let mut wechat = crate::wechat::WeChat::default();
        let chat_rooms = crate::wechat::exec_db_query(
            &mut wechat,
            String::from("MicroMsg.db"),
            String::from("select * from Contact"),
        )
        .unwrap();
        for item in chat_rooms.into_iter() {
            let fields = item.fields;
            println!("-------分割线-------");
            for field in fields.into_iter() {
                if field.column.eq("Reserved7") || field.column.eq("ExtraBuf") {
                    let c = hex::encode(&field.content);
                    print!("{}: {:?}, ", field.column, c);
                } else {
                    print!(
                        "{}: {}, ",
                        field.column,
                        String::from_utf8(field.content).unwrap()
                    );
                }
            }
            println!();
        }
    }

    #[test]
    fn test_exec_db_query_chatroom() {
        pub mod roomdata {
            include!("proto/roomdata.rs");
        }
        use prost::Message;

        let mut wechat = crate::wechat::WeChat::default();
        let chat_rooms = crate::wechat::exec_db_query(
            &mut wechat,
            String::from("MicroMsg.db"),
            String::from("select * from ChatRoom where ChatRoomName = '21262247140@chatroom'"),
        )
        .unwrap();
        for item in chat_rooms.into_iter() {
            let fields = item.fields;
            for field in fields.into_iter() {
                if field.column.eq("RoomData") {
                    let c = roomdata::RoomData::decode(field.content.as_slice()).unwrap();
                    println!("{}: {:?}", field.column, c);
                } else {
                    println!(
                        "{}: {}",
                        field.column,
                        String::from_utf8(field.content).unwrap()
                    );
                }
            }
        }
    }

    #[test]
    fn test_query_chat_room_info() {
        let mut wechat = crate::wechat::WeChat::default();
        let chat_room =
            crate::wechat::query_chat_room_info(&mut wechat, String::from("48585852775@chatroom"))
                .unwrap();
        print!("chat_room: {:?}", chat_room);
    }

    #[test]
    fn test_exec_db_query_chatroom_info() {
        pub mod roomdata {
            include!("proto/roomdata.rs");
        }
        use prost::Message;

        let mut wechat = crate::wechat::WeChat::default();
        let chat_rooms = crate::wechat::exec_db_query(
            &mut wechat,
            String::from("MicroMsg.db"),
            String::from("select * from ChatRoomInfo where ChatRoomName = '21262247140@chatroom'"),
        )
        .unwrap();
        for item in chat_rooms.into_iter() {
            let fields = item.fields;
            for field in fields.into_iter() {
                if field.column.eq("RoomData") {
                    let c = roomdata::RoomData::decode(field.content.as_slice()).unwrap();
                    println!("{}: {:?}", field.column, c);
                } else {
                    println!(
                        "{}: {}",
                        field.column,
                        String::from_utf8(field.content).unwrap()
                    );
                }
            }
        }
    }

    #[test]
    fn test_exec_db_query_contact_head_img_url() {
        pub mod roomdata {
            include!("proto/roomdata.rs");
        }
        use prost::Message;

        let mut wechat = crate::wechat::WeChat::default();
        let chat_rooms = crate::wechat::exec_db_query(
            &mut wechat,
            String::from("MicroMsg.db"),
            String::from(
                "select * from ContactHeadImgUrl where usrName = '25984981691552386@openim'",
            ),
        )
        .unwrap();
        for item in chat_rooms.into_iter() {
            println!("-------分割线-------");
            let fields = item.fields;
            for field in fields.into_iter() {
                if field.column.eq("RoomData") {
                    let c = roomdata::RoomData::decode(field.content.as_slice()).unwrap();
                    println!("{}: {:?}", field.column, c);
                } else {
                    println!(
                        "{}: {}",
                        field.column,
                        String::from_utf8(field.content).unwrap()
                    );
                }
            }
        }
    }

    #[test]
    fn test_query_contact_info() {
        let mut wechat = crate::wechat::WeChat::default();
        let contact_info = crate::wechat::query_contact_info(
            &mut wechat,
            String::from("25984981691552386@openim"),
        )
        .unwrap();
        println!("contact_info: {:?}", contact_info);
    }

    #[test]
    fn test_get_self_wx_id() {
        let mut wechat = crate::wechat::WeChat::default();
        let wx_id = crate::wechat::get_self_wx_id(&mut wechat).unwrap().unwrap();
        println!("WxId: {}", wx_id);
    }

    #[test]
    fn test_get_contacts() {
        let mut wechat = crate::wechat::WeChat::default();
        let contacts = crate::wechat::get_contacts(&mut wechat).unwrap().unwrap();
        println!("WxId: {:?}", contacts);
    }

    #[test]
    fn test_send_text() {
        let mut wechat = crate::wechat::WeChat::default();
        let status = crate::wechat::send_text(
            &mut wechat,
            String::from("艾特测试"),
            String::from("21262247140@chatroom"),
            String::from("@jingmo0614"),
        )
        .unwrap();
        println!("Success: {}", status);
    }

    #[test]
    fn test_send_image() {
        use std::path::PathBuf;

        let mut wechat = crate::wechat::WeChat::default();
        let status = crate::wechat::send_image(
            &mut wechat,
            PathBuf::from("C:\\Users\\Administrator\\Pictures\\1.jpg"),
            String::from("filehelper"),
        )
        .unwrap();
        println!("Success: {}", status);
    }

    #[test]
    fn test_recv_msg() {
        let mut wechat = crate::wechat::WeChat::default();
        let mut socket = crate::wechat::enable_listen(&mut wechat).unwrap();
        for _index in 0..5 {
            // let _ = crate::wechat::refresh_pyq(0, &mut wechat);
            let msg = crate::wechat::recv_msg(&mut socket).unwrap();
            println!("WxMsg: {:?}", msg);
            println!("--------------------------------------------------");
        }
        let _ = crate::wechat::disable_listen(&mut wechat);
    }

    #[test]
    fn test_get_msg_types() {
        let mut wechat = crate::wechat::WeChat::default();
        let types = crate::wechat::get_msg_types(&mut wechat);
        println!("{:?}", types);
    }

    #[test]
    fn test_accept_new_friend() {
        let mut wechat = crate::wechat::WeChat::default();
        let v3 = String::from("v3_020b3826fd03010000000000d65613e9435fd2000000501ea9a3dba12f95f6b60a0536a1adb6b4e20a513856625d11892e0635fe745d9c7ee96937f341a860c34107c6417414e5b41e427fc3d26a6af2590a1f@stranger");
        let v4 = String::from("v4_000b708f0b0400000100000000003c3767b326120d5b5795b98031641000000050ded0b020927e3c97896a09d47e6e9eac7eea28e4a39b49644b3b702b82268c1d40370261e3ae6eb543d231fbd29ee7a326598ba810316c10171871103ad967ca4d147d9f6dd8fa5ccd4986042520a1173c8138e5afe21f795ee50fecf58b4ac5269acd80028627dbf65fd17ca57c0e479fbe0392288a6f42@stranger");
        let status = crate::wechat::accept_new_friend(v3, v4, 17, &mut wechat).unwrap();
        println!("Status: {}", status);
    }

    #[test]
    fn test_add_chatroom_members() {
        let mut wechat = crate::wechat::WeChat::default();
        let status = crate::wechat::add_chatroom_members(
            String::from("*****@chatroom"),
            String::from("****"),
            &mut wechat,
        )
        .unwrap();
        println!("Status: {}", status);
    }

    #[test]
    fn test_del_chatroom_members() {
        let mut wechat = crate::wechat::WeChat::default();
        let status = crate::wechat::del_chatroom_members(
            String::from("34476879773@chatroom"),
            String::from("*******"),
            &mut wechat,
        )
        .unwrap();
        println!("Status: {}", status);
    }

    #[test]
    fn test_get_user_info() {
        let mut wechat = crate::wechat::WeChat::default();
        let user_info = crate::wechat::get_user_info(&mut wechat).unwrap();
        println!("UserInfo: {:?}", user_info);
    }

    #[test]
    fn test_recv_transfer() {
        let mut wechat = crate::wechat::WeChat::default();
        let status = crate::wechat::recv_transfer(
            String::from("****"),
            String::from("1000050001202306300415889890620"),
            String::from("100005000123063000081247810011296088"),
            &mut wechat,
        )
        .unwrap();
        println!("Status: {}", status);
    }

    #[test]
    fn test_decrypt_image() {
        let mut wechat = crate::wechat::WeChat::default();
        let status = crate::wechat::decrypt_image(
            String::from("C:\\Users\\Administrator\\Documents\\WeChat Files\\****\\FileStorage\\MsgAttach\\c963b851e0578c320c2966c6fc49e35c\\Image\\2023-05\\c66044e188c64452e236e53eff73324b.dat"),
            String::from("C:\\foo"),
            &mut wechat,
        )
        .unwrap();
        println!("Status: {}", status);
    }

    #[test]
    fn test_attach_msg() {
        let mut wechat = crate::wechat::WeChat::default();
        let status =
            crate::wechat::attach_msg(1, String::from(""), String::from(""), &mut wechat).unwrap();
        println!("Status: {}", status);
    }

    #[test]
    fn test_get_audio_msg() {
        let mut wechat = crate::wechat::WeChat::default();
        let status =
            crate::wechat::get_audio_msg(7072630968956565993, String::from("C:/"), &mut wechat)
                .unwrap();
        println!("Status: {}", status);
    }

    #[test]
    fn test_send_pat_msg() {
        let mut wechat = crate::wechat::WeChat::default();
        let status = crate::wechat::sned_pat_msg(
            String::from("21262247140@chatroom"),
            String::from("jingmo0614"),
            &mut wechat,
        )
        .unwrap();
        println!("Status: {}", status);
    }

    #[test]
    fn test_exec_ocr() {
        use std::path::PathBuf;

        let mut wechat = crate::wechat::WeChat::default();
        let ocr_msg = crate::wechat::exec_ocr(
            PathBuf::from("C:\\Users\\Administrator\\Pictures\\5.png"),
            &mut wechat,
        )
        .unwrap()
        .unwrap();
        println!("ocr: {:?}", ocr_msg);
    }

    #[test]
    fn test_forward_msg() {
        let mut wechat = crate::wechat::WeChat::default();
        let status = crate::wechat::forward_msg(
            5744142522200397761,
            String::from("21262247140@chatroom"),
            &mut wechat,
        )
        .unwrap();
        println!("Status: {}", status);
    }
}
