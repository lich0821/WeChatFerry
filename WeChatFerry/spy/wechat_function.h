#include <string>
#include <vector>
#include <windows.h>

namespace offset {
    const UINT64 kGetAccountServiceMgr = 0x1B50D00; //ok 
    const UINT64 kGetCurrentDataPath = 0x2248D40; //ok
    const UINT64 kGetAppDataSavePath = 0x25DBFE0; //ok 
    const UINT64 kGetSendMessageMgr = 0x1B4F500; //OK
    const UINT64 kNewChatMsgByDownloadMgr = 0x1B59670; //ok 
    const UINT64 kSendTextMsg = 0x22C2070; //OK
    const UINT64 kFreeChatMsg = 0x1B50D80; //OK

    const UINT64 kDoAddMsg = 0x230A490; //ok
    const UINT64 kSendImageMsg = 0x22B7800; //ok
    const UINT64 kChatMsgInstanceCounter = kNewChatMsgByDownloadMgr; //ok 
    const UINT64 kSendFileMsg = 0x20CB750;//ok
    const UINT64 kGetAppMsgMgr = 0x1B544A0; //ok 
    const UINT64 kGetContactMgr = 0x1B3CCD0;//ok
    const UINT64 kGetContactList = 0x219A220;//ok 

    const UINT64 k_sqlite3_exec = 0x3A59B40;//ok 
    const UINT64 k_sqlite3_prepare = 0x3A617F0;//ok 
    //const UINT64 k_sqlite3_open = 0x27242a0; //1 
    const UINT64 k_sqlite3_step = 0x3A1DB40;//ok 
    const UINT64 k_sqlite3_column_count = 0x3A1E360;//ok 
    const UINT64 k_sqlite3_column_name = 0x3A1ED60;//ok 
    const UINT64 k_sqlite3_column_type = 0x3A1EBB0;//ok
    const UINT64 k_sqlite3_column_blob = 0x3A1E390;//ok
    const UINT64 k_sqlite3_column_bytes = 0x3A1E480;//ok 
    const UINT64 k_sqlite3_finalize = 0x3A1CBF0; //ok

    const UINT64 kGPInstance = 0x58DD340; //ok

    const UINT64 kMicroMsgDB = 0xb8; //ok
    const UINT64 kChatMsgDB = 0x2c8; //ok
    const UINT64 kMiscDB = 0x5f0; //ok
    const UINT64 kEmotionDB = 0x15f0; //ok
    const UINT64 kMediaDB = 0xF48; //ok
    const UINT64 kBizchatMsgDB = 0x1AC0;//ok

    const UINT64 kFunctionMsgDB = 0x1b98;//ok
    const UINT64 kDBName = 0x28;
    const UINT64 kStorageStart = 0x0;
    const UINT64 kStorageEnd = 0x0;


    const UINT64 kMultiDBMgr = 0x593AC38; //ok 
    const UINT64 kPublicMsgMgr = 0x59381D8; //ok 
    const UINT64 kFavoriteStorageMgr = 0x593B7D0; //ok 


    const UINT64 kChatRoomMgr = 0x1B7EEC0; //ok 
    const UINT64 kGetChatRoomDetailInfo = 0x2160C10; //ok 
    const UINT64 kNewChatRoomInfo = 0x25051D0;//ok 
    const UINT64 kFreeChatRoomInfo = 0x25053B0;//ok 
    //const UINT64 kDoAddMemberToChatRoom = 0xe63c70;
    //const UINT64 kDoModChatRoomMemberNickName = 0xe6db00;
    //const UINT64 kDelMemberFromChatRoom = 0xe64290;
    const UINT64 kGetMemberFromChatRoom = 0x2162460;//ok 
    const UINT64 kNewChatRoom = 0x25025F0;//ok 
    const UINT64 kFreeChatRoom = 0x25027F0;//ok 

    //const UINT64 kTopMsg = 0xa5e4f0;
    //const UINT64 kRemoveTopMsg = 0xe787b0;
    //const UINT64 kInviteMember = 0xe63650;
    //const UINT64 kHookLog = 0x1304e60;

    //const UINT64 kCreateChatRoom = 0xe63340;
    //const UINT64 kQuitChatRoom = 0xe6e3b0;
    const UINT64 kForwardMsg = 0x22C15F0; //ok 

    //const UINT64 kOnSnsTimeLineSceneFinish = 0x1a73150;
    //const UINT64 kSNSGetFirstPage = 0x1a51dd0;
    //const UINT64 kSNSGetNextPageScene = 0x1a77240;
    //const UINT64 kSNSDataMgr = 0xeebda0;
    //const UINT64 kSNSTimeLineMgr = 0x19e83a0;
    const UINT64 kGetMgrByPrefixLocalId = 0x213B010; //ok 
    //const UINT64 kAddFavFromMsg = 0x1601520;
    const UINT64 kGetChatMgr = 0x1B82BF0; //ok 
    //const UINT64 kGetFavoriteMgr = 0x8c69b0;
    //const UINT64 kAddFavFromImage = 0x160b920;
    const UINT64 kGetContact = 0x2194630; //ok
    const UINT64 kNewContact = 0x25193B0; //ok 
    const UINT64 kFreeContact = 0x2519A60; //ok 
    //const UINT64 kNewMMReaderItem = 0x8c79a0;
    //const UINT64 kFreeMMReaderItem = 0x8c6da0;
    //const UINT64 kForwordPublicMsg = 0xddc6c0;
    const UINT64 kParseAppMsgXml = 0x24B3FD0; //ok 
    const UINT64 kNewAppMsgInfo = 0x1BCE7B0; //ok 
    const UINT64 kFreeAppMsgInfo = 0x1B93D80; //ok 
    const UINT64 kGetPreDownLoadMgr = 0x1C0A3A0; //ok 
    const UINT64 kPushAttachTask = 0x1CDA9B0; //ok
    //const UINT64 kGetCustomSmileyMgr = 0x915c00;
    //const UINT64 kSendCustomEmotion = 0xec0a40;
    //const UINT64 kNewJsApiShareAppMessage = 0x13be1a0;
    //const UINT64 kInitJsConfig = 0x137bc00;
    //const UINT64 kSendApplet = 0x13c0920;
    //const UINT64 kSendAppletSecond = 0x13c1150;
    //const UINT64 kGetAppInfoByWaid = 0x13c5790;
    //const UINT64 kCopyShareAppMessageRequest = 0x13c0670;
    //const UINT64 kNewWAUpdatableMsgInfo = 0x919ca0;
    //const UINT64 kFreeWAUpdatableMsgInfo = 0x8fc230;
    //const UINT64 kSendPatMsg = 0x195f340;
    //const UINT64 kGetOCRManager = 0x999780;
    //const UINT64 kDoOCRTask = 0x190b2a0;

    const UINT64 kGetLockWechatMgr = 0x1C84DA0;//ok 
    const UINT64 kRequestLockWechat = 0x1C39860;//ok 
    const UINT64 kRequestUnLockWechat = 0x1C39B00;//ok 

    const UINT64 kOnLoginBtnClick = 0x202BC90;//ok 
    const UINT64 kOnLoginBtnParam = 0x4ECEE08;//ok

    const UINT64 kGetQRCodeLoginMgr = 0x201E420;//ok 

    const UINT64 kUpdateMsg = 0x2142200;//ok 
    const UINT64 kGetVoiceMgr = 0x1E13320;//ok 
    const UINT64 kChatMsg2NetSceneSendMsg = 0x1B71FD0;//ok
    const UINT64 kTranslateVoice = 0x2353E00;//ok 
    const UINT64 kNewWebViewPageConfig = 0x1B53AE0; //ok
    const UINT64 kFreeWebViewPageConfig = 0x1B53D10; //ok 
    const UINT64 kGetWebViewMgr = 0x1B43950; //ok

    const UINT64 kShowWebView = 0x302ED40;//ok 
    const UINT64 kSetUrl = 0x26155F0; //ok 



    //发送小程序
    const UINT64 kNewJsApiShareAppMessage = 0x26CDA30; //ok
    const UINT64 kInitJsConfig = 0x268A970; //ok
    const UINT64 kSendApplet = 0x26D01D0; //ok
    const UINT64 kSendAppletSecond = 0x26D0A00; //ok
    const UINT64 kGetAppInfoByWaid = 0x26D4F80;  //ok
    const UINT64 kCopyShareAppMessageRequest = 0x26CFF20;//ok
    const UINT64 kNewWAUpdatableMsgInfo = 0x1BCDD10; //ok
    const UINT64 kFreeWAUpdatableMsgInfo = 0x1B92AC0;//ok
    const UINT64 kSendPatMsg = 0x2CA97A0;//ok PatMgr::SendPatMsg
    const UINT64 kSendAppletRcxParam = 0x4F64A60; //ok 

    //取群联系人昵称
    const UINT64 kChatRoomNickNameMgr = 0x1B7F100; //ok
    const UINT64 kGetChatRoomNickName = 0x21625D0; //ok



    //卡片
    //const UINT64 kRichTextMgr = 0x1C23630;
    const UINT64 kSendRichTextMsg = 0x20D5730;
    const UINT64 kNewRChatMsg = 0x1B58BC0;
    const UINT64 kFreeRChatMsg = 0x1B57F90;

    //HOOK偏移
    const UINT64 wcf_hook = 0x00;    // Hook地址
    const UINT64 wcf_HookCall = 0x213A2A0;    // Call地址
    const UINT64 wcf_msgId = 0x30;   // 消息ID地址
    const UINT64 wcf_type = 0x38;    // 消息类型地址
    const UINT64 wcf_isSelf = 0x3C;  // 是否自己发送标志地址
    const UINT64 wcf_ts = 0x44;      // TimeStamp
    const UINT64 wcf_roomId = 0x48;  // 群聊时，为群ID；私聊时，为微信ID
    const UINT64 wcf_content = 0x88; // 消息内容地址
    const UINT64 wcf_wxid = 0x240;    // 私聊时，为空；群聊时，为发送者微信ID
    const UINT64 wcf_sign = 0x260;    // Sign
    const UINT64 wcf_thumb = 0x280;   // 缩略图
    const UINT64 wcf_extra = 0x2A0;   // 附加数据
    const UINT64 wcf_msgXml = 0x308;  // 消息xml内容地址

    //登录状态
    const UINT64 wcf_kLoginStatu = 0x59380B0;
    const UINT64 wcf_iwxid = 0x5AB7FB8;
    const UINT64 wcf_nickName = 0x5AB8098;
    const UINT64 wcf_mobile = 0x5AB7FD8;
    const UINT64 wcf_home = 0x5A7E190;


}  // namespace offset
