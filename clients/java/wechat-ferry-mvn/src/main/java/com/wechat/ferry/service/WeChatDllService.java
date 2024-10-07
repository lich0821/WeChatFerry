package com.wechat.ferry.service;

import java.util.List;

import com.wechat.ferry.entity.vo.request.WxPpWcfDatabaseSqlReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfDatabaseTableReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfGroupMemberReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfPatOnePatMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfSendEmojiMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfSendFileMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfSendImageMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfSendRichTextMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfSendTextMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpWcfSendXmlMsgReq;
import com.wechat.ferry.entity.vo.response.WxPpWcfContactsResp;
import com.wechat.ferry.entity.vo.response.WxPpWcfDatabaseRowResp;
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
     * 获取所有消息类型
     *
     * @return 消息类型列表
     *
     * @author chandler
     * @date 2024-10-01 21:22
     */
    List<WxPpWcfMsgTypeResp> queryMsgTypeList();

    /**
     * 获取所有联系人
     *
     * @return 联系人列表
     *
     * @author chandler
     * @date 2024-10-02 16:59
     */
    List<WxPpWcfContactsResp> queryContactsList();

    /**
     * 获取可查询数据库
     * 
     * @param request 请求入参
     * @return 数据库记录
     * 
     * @author chandler
     * @date 2024-10-02 17:52
     */
    List<WxPpWcfDatabaseRowResp> queryDatabaseSql(WxPpWcfDatabaseSqlReq request);

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
    List<String> queryDatabaseTable(WxPpWcfDatabaseTableReq request);

    /**
     * 查询群成员
     *
     * @param request 请求入参
     * @return 数据库记录
     *
     * @author chandler
     * @date 2024-10-02 20:59
     */
    List<WxPpWcfGroupMemberResp> queryGroupMember(WxPpWcfGroupMemberReq request);

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
     * 拍一拍
     *
     * @param request 请求入参
     * @return 消息发送返回
     *
     * @author chandler
     * @date 2024-10-06 15:54
     */
    WxPpWcfSendPatOnePatMsgResp patOnePat(WxPpWcfPatOnePatMsgReq request);

}
