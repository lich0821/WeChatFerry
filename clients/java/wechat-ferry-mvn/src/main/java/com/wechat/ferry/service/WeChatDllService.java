package com.wechat.ferry.service;

import java.util.List;

import com.wechat.ferry.entity.vo.request.WxPpDatabaseSqlReq;
import com.wechat.ferry.entity.vo.request.WxPpDatabaseTableReq;
import com.wechat.ferry.entity.vo.request.WxPpGroupMemberReq;
import com.wechat.ferry.entity.vo.request.WxPpPatOnePatMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpSendEmojiMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpSendFileMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpSendImageMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpSendRichTextMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpSendTextMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpSendXmlMsgReq;
import com.wechat.ferry.entity.vo.response.WxPpContactsResp;
import com.wechat.ferry.entity.vo.response.WxPpDatabaseRowResp;
import com.wechat.ferry.entity.vo.response.WxPpGroupMemberResp;
import com.wechat.ferry.entity.vo.response.WxPpLoginInfoResp;
import com.wechat.ferry.entity.vo.response.WxPpMsgTypeResp;
import com.wechat.ferry.entity.vo.response.WxPpSendEmojiMsgResp;
import com.wechat.ferry.entity.vo.response.WxPpSendFileMsgResp;
import com.wechat.ferry.entity.vo.response.WxPpSendImageMsgResp;
import com.wechat.ferry.entity.vo.response.WxPpSendPatOnePatMsgResp;
import com.wechat.ferry.entity.vo.response.WxPpSendRichTextMsgResp;
import com.wechat.ferry.entity.vo.response.WxPpSendTextMsgResp;
import com.wechat.ferry.entity.vo.response.WxPpSendXmlMsgResp;

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
    WxPpLoginInfoResp queryLoginWeChatInfo();

    /**
     * 获取所有消息类型
     *
     * @return 消息类型列表
     *
     * @author chandler
     * @date 2024-10-01 21:22
     */
    List<WxPpMsgTypeResp> queryMsgTypeList();

    /**
     * 获取所有联系人
     *
     * @return 联系人列表
     *
     * @author chandler
     * @date 2024-10-02 16:59
     */
    List<WxPpContactsResp> queryContactsList();

    /**
     * 获取可查询数据库
     * 
     * @param request 请求入参
     * @return 数据库记录
     * 
     * @author chandler
     * @date 2024-10-02 17:52
     */
    List<WxPpDatabaseRowResp> queryDatabaseSql(WxPpDatabaseSqlReq request);

    /**
     * 获取数据库所有表名称
     *
     * @return 数据库名称列表
     *
     * @author chandler
     * @date 2024-10-02 17:53
     */
    List<String> queryDatabaseAllTableName();

    /**
     * 获取数据库表
     *
     * @param request 请求入参
     * @return 数据库记录
     *
     * @author chandler
     * @date 2024-10-02 17:52
     */
    List<String> queryDatabaseTable(WxPpDatabaseTableReq request);

    /**
     * 查询群成员
     *
     * @param request 请求入参
     * @return 数据库记录
     *
     * @author chandler
     * @date 2024-10-02 20:59
     */
    List<WxPpGroupMemberResp> queryGroupMember(WxPpGroupMemberReq request);

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
    WxPpSendTextMsgResp sendTextMsg(WxPpSendTextMsgReq request);

    /**
     * 发送图片消息
     *
     * @param request 请求入参
     * @return 消息发送返回
     *
     * @author chandler
     * @date 2024-10-04 23:06
     */
    WxPpSendImageMsgResp sendImageMsg(WxPpSendImageMsgReq request);

    /**
     * 发送文件消息
     *
     * @param request 请求入参
     * @return 消息发送返回
     *
     * @author chandler
     * @date 2024-10-04 23:15
     */
    WxPpSendFileMsgResp sendFileMsg(WxPpSendFileMsgReq request);

    /**
     * 发送XML消息
     *
     * @param request 请求入参
     * @return 消息发送返回
     *
     * @author chandler
     * @date 2024-10-04 23:15
     */
    WxPpSendXmlMsgResp sendXmlMsg(WxPpSendXmlMsgReq request);

    /**
     * 发送表情消息
     *
     * @param request 请求入参
     * @return 消息发送返回
     *
     * @author chandler
     * @date 2024-10-04 23:29
     */
    WxPpSendEmojiMsgResp sendEmojiMsg(WxPpSendEmojiMsgReq request);

    /**
     * 发送富文本消息
     *
     * @param request 请求入参
     * @return 消息发送返回
     *
     * @author chandler
     * @date 2024-10-06 15:48
     */
    WxPpSendRichTextMsgResp sendRichTextMsg(WxPpSendRichTextMsgReq request);

    /**
     * 拍一拍
     *
     * @param request 请求入参
     * @return 消息发送返回
     *
     * @author chandler
     * @date 2024-10-06 15:54
     */
    WxPpSendPatOnePatMsgResp patOnePat(WxPpPatOnePatMsgReq request);

}
