package com.wechat.ferry.service;

import java.util.List;

import com.wechat.ferry.entity.vo.request.WxPpWcfAddFriendGroupMemberReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfDatabaseSqlReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfDatabaseTableReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfDeleteGroupMemberReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfGroupMemberReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfInviteGroupMemberReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfPassFriendApplyReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfPatOnePatMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfReceiveTransferReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfRevokeMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfSendEmojiMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfSendFileMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfSendImageMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfSendRichTextMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfSendTextMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfSendXmlMsgReq;
import com.wechat.ferry.entity.vo.response.WxPpWcfContactsResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfDatabaseRowResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfDatabaseTableResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfGroupMemberResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfLoginInfoResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfMsgTypeResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfSendEmojiMsgResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfSendFileMsgResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfSendImageMsgResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfSendPatOnePatMsgResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfSendRichTextMsgResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfSendTextMsgResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfSendXmlMsgResp;

/**
 * 业务接口-对接原本DLL的接口
 *
 * @author chandler
 * @date 2024-10-01 15:57
 */
public interface WeChatDllService {

    /**
     * 当前微信客户端是否登录微信号
     *
     * @return true-已登录 false-未登录
     * 
     * @author chandler
     * @date 2024-10-01 21:20
     */
    Boolean loginStatus();

    /**
     * 获取登录微信内部识别号UID
     * 获得微信客户端登录的微信ID
     *
     * @return 微信内部识别号UID
     *
     * @author chandler
     * @date 2024-10-01 21:22
     */
    String queryLoginWeChatUid();

    /**
     * 获取登录微信信息
     *
     * @return 当前登录微信信息
     *
     * @author chandler
     * @date 2024-10-05 22:54
     */
    WxPpWcfLoginInfoResp queryLoginWeChatInfo();

    /**
     * 获取消息类型列表
     *
     * @return 消息类型列表
     *
     * @author chandler
     * @date 2024-10-01 21:22
     */
    List<WxPpWcfMsgTypeResp> queryMsgTypeList();

    /**
     * 获取联系人列表
     *
     * @return 联系人列表
     *
     * @author chandler
     * @date 2024-10-02 16:59
     */
    List<WxPpWcfContactsResp> queryContactsList();

    /**
     * 获取数据库表名称列表
     *
     * @return 数据库名称列表
     *
     * @author chandler
     * @date 2024-10-02 17:53
     */
    List<String> queryDbTableNameList();

    /**
     * 获取指定数据库中的所有表
     *
     * @param request 请求入参
     * @return 数据库记录
     *
     * @author chandler
     * @date 2024-10-02 17:52
     */
    List<WxPpWcfDatabaseTableResp> queryDbTableList(WxPpWcfDatabaseTableReq request);

    /**
     * 执行数据库查询SQL
     * 
     * @param request 请求入参
     * @return 数据库记录
     * 
     * @author chandler
     * @date 2024-10-02 17:52
     */
    List<WxPpWcfDatabaseRowResp> execDbQuerySql(WxPpWcfDatabaseSqlReq request);

    /**
     * 发送文本消息（可 @）
     *
     * @param request 请求入参
     * @return 消息发送返回
     *
     * @example sendText(" Hello @ 某人1 @ 某人2 ", " xxxxxxxx @ chatroom ", "wxid_xxxxxxxxxxxxx1,wxid_xxxxxxxxxxxxx2");
     *
     * @author chandler
     * @date 2024-10-02 20:40
     */
    WxPpWcfSendTextMsgResp sendTextMsg(WxPpWcfSendTextMsgReq request);

    /**
     * 发送富文本消息
     *
     * @param request 请求入参
     * @return 消息发送返回
     *
     * @author chandler
     * @date 2024-10-06 15:48
     */
    WxPpWcfSendRichTextMsgResp sendRichTextMsg(WxPpWcfSendRichTextMsgReq request);

    /**
     * 发送XML消息
     *
     * @param request 请求入参
     * @return 消息发送返回
     *
     * @author chandler
     * @date 2024-10-04 23:15
     */
    WxPpWcfSendXmlMsgResp sendXmlMsg(WxPpWcfSendXmlMsgReq request);

    /**
     * 发送图片消息
     *
     * @param request 请求入参
     * @return 消息发送返回
     *
     * @author chandler
     * @date 2024-10-04 23:06
     */
    WxPpWcfSendImageMsgResp sendImageMsg(WxPpWcfSendImageMsgReq request);

    /**
     * 发送表情消息
     *
     * @param request 请求入参
     * @return 消息发送返回
     *
     * @author chandler
     * @date 2024-10-04 23:29
     */
    WxPpWcfSendEmojiMsgResp sendEmojiMsg(WxPpWcfSendEmojiMsgReq request);

    /**
     * 发送文件消息
     *
     * @param request 请求入参
     * @return 消息发送返回
     *
     * @author chandler
     * @date 2024-10-04 23:15
     */
    WxPpWcfSendFileMsgResp sendFileMsg(WxPpWcfSendFileMsgReq request);

    /**
     * 拍一拍
     *
     * @param request 请求入参
     * @return 消息发送返回
     *
     * @author chandler
     * @date 2024-10-06 15:54
     */
    WxPpWcfSendPatOnePatMsgResp patOnePat(WxPpWcfPatOnePatMsgReq request);

    /**
     * 撤回消息
     *
     * @return 结果状态
     *
     * @author chandler
     * @date 2024-12-25 11:59
     */
    String revokeMsg(WxPpWcfRevokeMsgReq request);

    /**
     * 通过好友申请
     *
     * @param request 请求入参
     * @return 结果状态
     *
     * @author chandler
     * @date 2024-12-25 09:38
     */
    String passFriendApply(WxPpWcfPassFriendApplyReq request);

    /**
     * 添加群成员为微信好友
     *
     * @param request 请求入参
     * @return 结果状态
     *
     * @author chandler
     * @date 2024-12-25 09:38
     */
    String addFriendGroupMember(WxPpWcfAddFriendGroupMemberReq request);

    /**
     * 查询群成员列表
     *
     * @param request 请求入参
     * @return 数据库记录
     *
     * @author chandler
     * @date 2024-10-02 20:59
     */
    List<WxPpWcfGroupMemberResp> queryGroupMemberList(WxPpWcfGroupMemberReq request);

    /**
     * 邀请群成员
     *
     * @param request 请求入参
     * @return 结果状态
     *
     * @author chandler
     * @date 2024-12-25 10:02
     */
    String inviteGroupMember(WxPpWcfInviteGroupMemberReq request);

    /**
     * 删除群成员
     *
     * @param request 请求入参
     * @return 结果状态
     *
     * @author chandler
     * @date 2024-12-25 10:03
     */
    String deleteGroupMember(WxPpWcfDeleteGroupMemberReq request);

    /**
     * 查询朋友圈
     *
     * @return 结果状态
     *
     * @author chandler
     * @date 2024-12-25 11:11
     */
    String queryFriendCircle();

    /**
     * 接收转账
     *
     * @param request 请求入参
     * @return 结果状态
     *
     * @author chandler
     * @date 2024-12-25 13:48
     */
    String receiveTransfer(WxPpWcfReceiveTransferReq request);

}
