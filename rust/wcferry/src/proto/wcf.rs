#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct Request {
    #[prost(enumeration = "Functions", tag = "1")]
    pub func: i32,
    #[prost(oneof = "request::Msg", tags = "2, 3, 4, 5, 6, 7, 8, 9, 10, 11")]
    pub msg: ::core::option::Option<request::Msg>,
}
/// Nested message and enum types in `Request`.
pub mod request {
    #[allow(clippy::derive_partial_eq_without_eq)]
    #[derive(Clone, PartialEq, ::prost::Oneof)]
    pub enum Msg {
        #[prost(message, tag = "2")]
        Empty(super::Empty),
        #[prost(string, tag = "3")]
        Str(::prost::alloc::string::String),
        #[prost(message, tag = "4")]
        Txt(super::TextMsg),
        #[prost(message, tag = "5")]
        File(super::PathMsg),
        #[prost(message, tag = "6")]
        Query(super::DbQuery),
        #[prost(message, tag = "7")]
        V(super::Verification),
        #[prost(message, tag = "8")]
        M(super::AddMembers),
        #[prost(message, tag = "9")]
        Xml(super::XmlMsg),
        #[prost(message, tag = "10")]
        Dec(super::DecPath),
        #[prost(message, tag = "11")]
        Tf(super::Transfer),
    }
}
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct Response {
    #[prost(enumeration = "Functions", tag = "1")]
    pub func: i32,
    #[prost(oneof = "response::Msg", tags = "2, 3, 4, 5, 6, 7, 8, 9, 10")]
    pub msg: ::core::option::Option<response::Msg>,
}
/// Nested message and enum types in `Response`.
pub mod response {
    #[allow(clippy::derive_partial_eq_without_eq)]
    #[derive(Clone, PartialEq, ::prost::Oneof)]
    pub enum Msg {
        /// Int 状态，通用
        #[prost(int32, tag = "2")]
        Status(i32),
        /// 字符串
        #[prost(string, tag = "3")]
        Str(::prost::alloc::string::String),
        /// 微信消息
        #[prost(message, tag = "4")]
        Wxmsg(super::WxMsg),
        /// 消息类型
        #[prost(message, tag = "5")]
        Types(super::MsgTypes),
        /// 联系人
        #[prost(message, tag = "6")]
        Contacts(super::RpcContacts),
        /// 数据库列表
        #[prost(message, tag = "7")]
        Dbs(super::DbNames),
        /// 表列表
        #[prost(message, tag = "8")]
        Tables(super::DbTables),
        /// 行列表
        #[prost(message, tag = "9")]
        Rows(super::DbRows),
        /// 个人信息
        #[prost(message, tag = "10")]
        Ui(super::UserInfo),
    }
}
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct Empty {}
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct WxMsg {
    /// 是否自己发送的
    #[prost(bool, tag = "1")]
    pub is_self: bool,
    /// 是否群消息
    #[prost(bool, tag = "2")]
    pub is_group: bool,
    /// 消息类型
    #[prost(int32, tag = "3")]
    pub r#type: i32,
    /// 消息 id
    #[prost(string, tag = "4")]
    pub id: ::prost::alloc::string::String,
    /// 消息 xml
    #[prost(string, tag = "5")]
    pub xml: ::prost::alloc::string::String,
    /// 消息发送者
    #[prost(string, tag = "6")]
    pub sender: ::prost::alloc::string::String,
    /// 群 id（如果是群消息的话）
    #[prost(string, tag = "7")]
    pub roomid: ::prost::alloc::string::String,
    /// 消息内容
    #[prost(string, tag = "8")]
    pub content: ::prost::alloc::string::String,
    /// 缩略图
    #[prost(string, tag = "9")]
    pub thumb: ::prost::alloc::string::String,
    /// 附加内容
    #[prost(string, tag = "10")]
    pub extra: ::prost::alloc::string::String,
}
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct TextMsg {
    /// 要发送的消息内容
    #[prost(string, tag = "1")]
    pub msg: ::prost::alloc::string::String,
    /// 消息接收人，当为群时可@
    #[prost(string, tag = "2")]
    pub receiver: ::prost::alloc::string::String,
    /// 要@的人列表，逗号分隔
    #[prost(string, tag = "3")]
    pub aters: ::prost::alloc::string::String,
}
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct PathMsg {
    /// 要发送的图片的路径
    #[prost(string, tag = "1")]
    pub path: ::prost::alloc::string::String,
    /// 消息接收人
    #[prost(string, tag = "2")]
    pub receiver: ::prost::alloc::string::String,
}
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct XmlMsg {
    /// 消息接收人
    #[prost(string, tag = "1")]
    pub receiver: ::prost::alloc::string::String,
    /// xml 内容
    #[prost(string, tag = "2")]
    pub content: ::prost::alloc::string::String,
    /// 图片路径
    #[prost(string, tag = "3")]
    pub path: ::prost::alloc::string::String,
    /// 消息类型
    #[prost(int32, tag = "4")]
    pub r#type: i32,
}
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct MsgTypes {
    #[prost(map = "int32, string", tag = "1")]
    pub types: ::std::collections::HashMap<i32, ::prost::alloc::string::String>,
}
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct RpcContact {
    /// 微信 id
    #[prost(string, tag = "1")]
    pub wxid: ::prost::alloc::string::String,
    /// 微信号
    #[prost(string, tag = "2")]
    pub code: ::prost::alloc::string::String,
    /// 备注
    #[prost(string, tag = "3")]
    pub remark: ::prost::alloc::string::String,
    /// 微信昵称
    #[prost(string, tag = "4")]
    pub name: ::prost::alloc::string::String,
    /// 国家
    #[prost(string, tag = "5")]
    pub country: ::prost::alloc::string::String,
    /// 省/州
    #[prost(string, tag = "6")]
    pub province: ::prost::alloc::string::String,
    /// 城市
    #[prost(string, tag = "7")]
    pub city: ::prost::alloc::string::String,
    /// 性别
    #[prost(int32, tag = "8")]
    pub gender: i32,
}
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct RpcContacts {
    #[prost(message, repeated, tag = "1")]
    pub contacts: ::prost::alloc::vec::Vec<RpcContact>,
}
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct DbNames {
    #[prost(string, repeated, tag = "1")]
    pub names: ::prost::alloc::vec::Vec<::prost::alloc::string::String>,
}
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct DbTable {
    /// 表名
    #[prost(string, tag = "1")]
    pub name: ::prost::alloc::string::String,
    /// 建表 SQL
    #[prost(string, tag = "2")]
    pub sql: ::prost::alloc::string::String,
}
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct DbTables {
    #[prost(message, repeated, tag = "1")]
    pub tables: ::prost::alloc::vec::Vec<DbTable>,
}
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct DbQuery {
    /// 目标数据库
    #[prost(string, tag = "1")]
    pub db: ::prost::alloc::string::String,
    /// 查询 SQL
    #[prost(string, tag = "2")]
    pub sql: ::prost::alloc::string::String,
}
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct DbField {
    /// 字段类型
    #[prost(int32, tag = "1")]
    pub r#type: i32,
    /// 字段名称
    #[prost(string, tag = "2")]
    pub column: ::prost::alloc::string::String,
    /// 字段内容
    #[prost(bytes = "vec", tag = "3")]
    pub content: ::prost::alloc::vec::Vec<u8>,
}
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct DbRow {
    #[prost(message, repeated, tag = "1")]
    pub fields: ::prost::alloc::vec::Vec<DbField>,
}
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct DbRows {
    #[prost(message, repeated, tag = "1")]
    pub rows: ::prost::alloc::vec::Vec<DbRow>,
}
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct Verification {
    /// 加密的用户名
    #[prost(string, tag = "1")]
    pub v3: ::prost::alloc::string::String,
    /// Ticket
    #[prost(string, tag = "2")]
    pub v4: ::prost::alloc::string::String,
    /// 添加方式：17 名片，30 扫码
    #[prost(int32, tag = "3")]
    pub scene: i32,
}
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct AddMembers {
    /// 要加的群ID
    #[prost(string, tag = "1")]
    pub roomid: ::prost::alloc::string::String,
    /// 要加群的人列表，逗号分隔
    #[prost(string, tag = "2")]
    pub wxids: ::prost::alloc::string::String,
}
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct UserInfo {
    /// 微信ID
    #[prost(string, tag = "1")]
    pub wxid: ::prost::alloc::string::String,
    /// 昵称
    #[prost(string, tag = "2")]
    pub name: ::prost::alloc::string::String,
    /// 手机号
    #[prost(string, tag = "3")]
    pub mobile: ::prost::alloc::string::String,
    /// 文件/图片等父路径
    #[prost(string, tag = "4")]
    pub home: ::prost::alloc::string::String,
}
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct DecPath {
    /// 源路径
    #[prost(string, tag = "1")]
    pub src: ::prost::alloc::string::String,
    /// 目标路径
    #[prost(string, tag = "2")]
    pub dst: ::prost::alloc::string::String,
}
#[allow(clippy::derive_partial_eq_without_eq)]
#[derive(Clone, PartialEq, ::prost::Message)]
pub struct Transfer {
    /// 转账人
    #[prost(string, tag = "1")]
    pub wxid: ::prost::alloc::string::String,
    /// 转账id transferid
    #[prost(string, tag = "2")]
    pub tid: ::prost::alloc::string::String,
}
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash, PartialOrd, Ord, ::prost::Enumeration)]
#[repr(i32)]
pub enum Functions {
    FuncReserved = 0,
    FuncIsLogin = 1,
    FuncGetSelfWxid = 16,
    FuncGetMsgTypes = 17,
    FuncGetContacts = 18,
    FuncGetDbNames = 19,
    FuncGetDbTables = 20,
    FuncGetUserInfo = 21,
    FuncSendTxt = 32,
    FuncSendImg = 33,
    FuncSendFile = 34,
    FuncSendXml = 35,
    FuncSendEmotion = 36,
    FuncEnableRecvTxt = 48,
    FuncDisableRecvTxt = 64,
    FuncExecDbQuery = 80,
    FuncAcceptFriend = 81,
    FuncAddRoomMembers = 82,
    FuncRecvTransfer = 83,
    FuncDecryptImage = 96,
}
impl Functions {
    /// String value of the enum field names used in the ProtoBuf definition.
    ///
    /// The values are not transformed in any way and thus are considered stable
    /// (if the ProtoBuf definition does not change) and safe for programmatic use.
    pub fn as_str_name(&self) -> &'static str {
        match self {
            Functions::FuncReserved => "FUNC_RESERVED",
            Functions::FuncIsLogin => "FUNC_IS_LOGIN",
            Functions::FuncGetSelfWxid => "FUNC_GET_SELF_WXID",
            Functions::FuncGetMsgTypes => "FUNC_GET_MSG_TYPES",
            Functions::FuncGetContacts => "FUNC_GET_CONTACTS",
            Functions::FuncGetDbNames => "FUNC_GET_DB_NAMES",
            Functions::FuncGetDbTables => "FUNC_GET_DB_TABLES",
            Functions::FuncGetUserInfo => "FUNC_GET_USER_INFO",
            Functions::FuncSendTxt => "FUNC_SEND_TXT",
            Functions::FuncSendImg => "FUNC_SEND_IMG",
            Functions::FuncSendFile => "FUNC_SEND_FILE",
            Functions::FuncSendXml => "FUNC_SEND_XML",
            Functions::FuncSendEmotion => "FUNC_SEND_EMOTION",
            Functions::FuncEnableRecvTxt => "FUNC_ENABLE_RECV_TXT",
            Functions::FuncDisableRecvTxt => "FUNC_DISABLE_RECV_TXT",
            Functions::FuncExecDbQuery => "FUNC_EXEC_DB_QUERY",
            Functions::FuncAcceptFriend => "FUNC_ACCEPT_FRIEND",
            Functions::FuncAddRoomMembers => "FUNC_ADD_ROOM_MEMBERS",
            Functions::FuncRecvTransfer => "FUNC_RECV_TRANSFER",
            Functions::FuncDecryptImage => "FUNC_DECRYPT_IMAGE",
        }
    }
    /// Creates an enum from field names used in the ProtoBuf definition.
    pub fn from_str_name(value: &str) -> ::core::option::Option<Self> {
        match value {
            "FUNC_RESERVED" => Some(Self::FuncReserved),
            "FUNC_IS_LOGIN" => Some(Self::FuncIsLogin),
            "FUNC_GET_SELF_WXID" => Some(Self::FuncGetSelfWxid),
            "FUNC_GET_MSG_TYPES" => Some(Self::FuncGetMsgTypes),
            "FUNC_GET_CONTACTS" => Some(Self::FuncGetContacts),
            "FUNC_GET_DB_NAMES" => Some(Self::FuncGetDbNames),
            "FUNC_GET_DB_TABLES" => Some(Self::FuncGetDbTables),
            "FUNC_GET_USER_INFO" => Some(Self::FuncGetUserInfo),
            "FUNC_SEND_TXT" => Some(Self::FuncSendTxt),
            "FUNC_SEND_IMG" => Some(Self::FuncSendImg),
            "FUNC_SEND_FILE" => Some(Self::FuncSendFile),
            "FUNC_SEND_XML" => Some(Self::FuncSendXml),
            "FUNC_SEND_EMOTION" => Some(Self::FuncSendEmotion),
            "FUNC_ENABLE_RECV_TXT" => Some(Self::FuncEnableRecvTxt),
            "FUNC_DISABLE_RECV_TXT" => Some(Self::FuncDisableRecvTxt),
            "FUNC_EXEC_DB_QUERY" => Some(Self::FuncExecDbQuery),
            "FUNC_ACCEPT_FRIEND" => Some(Self::FuncAcceptFriend),
            "FUNC_ADD_ROOM_MEMBERS" => Some(Self::FuncAddRoomMembers),
            "FUNC_RECV_TRANSFER" => Some(Self::FuncRecvTransfer),
            "FUNC_DECRYPT_IMAGE" => Some(Self::FuncDecryptImage),
            _ => None,
        }
    }
}
