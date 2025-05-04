package com.wechat.ferry.controller;

import java.util.List;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import com.wechat.ferry.entity.TResponse;
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
import com.wechat.ferry.enums.ResponseCodeEnum;
import com.wechat.ferry.service.WeChatDllService;

import io.swagger.annotations.Api;
import io.swagger.annotations.ApiOperation;
import lombok.extern.slf4j.Slf4j;

/**
 * 控制层-微信DLL处理
 *
 * @author chandler
 * @date 2024-10-01 15:48
 */
@Slf4j
@RestController
@RequestMapping("/wechat/cgi/wcf")
@Api(tags = "微信消息处理-接口")
public class WeChatDllController {

    private WeChatDllService weChatDllService;

    @Autowired
    public void setWeChatDllService(WeChatDllService weChatDllService) {
        this.weChatDllService = weChatDllService;
    }

    @ApiOperation(value = "查询登录状态", notes = "loginStatus")
    @PostMapping(value = "/loginStatus")
    public TResponse<Object> loginStatus() {
        Boolean status = weChatDllService.loginStatus();
        return TResponse.ok(ResponseCodeEnum.SUCCESS, status);
    }

    @ApiOperation(value = "获取登录微信号", notes = "queryLoginWeChatUid")
    @PostMapping(value = "/loginWeChatUid")
    public TResponse<Object> queryLoginWeChatUid() {
        String weChatUid = weChatDllService.queryLoginWeChatUid();
        return TResponse.ok(ResponseCodeEnum.SUCCESS, weChatUid);
    }

    @ApiOperation(value = "获取登录微信信息", notes = "queryLoginWeChatInfo")
    @PostMapping(value = "/loginWeChatInfo")
    public TResponse<WxPpWcfLoginInfoResp> queryLoginWeChatInfo() {
        WxPpWcfLoginInfoResp resp = weChatDllService.queryLoginWeChatInfo();
        return TResponse.ok(ResponseCodeEnum.SUCCESS, resp);
    }

    @ApiOperation(value = "获取消息类型列表", notes = "queryMsgTypeList")
    @PostMapping(value = "/list/msgType")
    public TResponse<List<WxPpWcfMsgTypeResp>> queryMsgTypeList() {
        List<WxPpWcfMsgTypeResp> list = weChatDllService.queryMsgTypeList();
        return TResponse.ok(ResponseCodeEnum.SUCCESS, list);
    }

    @ApiOperation(value = "获取联系人列表", notes = "queryContactsList")
    @PostMapping(value = "/list/contacts")
    public TResponse<List<WxPpWcfContactsResp>> queryContactsList() {
        List<WxPpWcfContactsResp> list = weChatDllService.queryContactsList();
        return TResponse.ok(ResponseCodeEnum.SUCCESS, list);
    }

    @ApiOperation(value = "获取数据库表名称列表", notes = "queryDbTableNameList")
    @PostMapping(value = "/list/dbTableName")
    public TResponse<List<String>> queryDbTableNameList() {
        List<String> list = weChatDllService.queryDbTableNameList();
        return TResponse.ok(ResponseCodeEnum.SUCCESS, list);
    }

    @ApiOperation(value = "获取指定数据库中的表列表", notes = "queryDbTableList")
    @PostMapping(value = "/list/dbTable")
    public TResponse<List<WxPpWcfDatabaseTableResp>> queryDbTableList(@Validated @RequestBody WxPpWcfDatabaseTableReq request) {
        List<WxPpWcfDatabaseTableResp> list = weChatDllService.queryDbTableList(request);
        return TResponse.ok(ResponseCodeEnum.SUCCESS, list);
    }

    @ApiOperation(value = "执行数据库查询SQL", notes = "execDbQuerySql")
    @PostMapping(value = "/exec/dbQuerySql")
    public TResponse<List<WxPpWcfDatabaseRowResp>> execDbQuerySql(@Validated @RequestBody WxPpWcfDatabaseSqlReq request) {
        List<WxPpWcfDatabaseRowResp> list = weChatDllService.execDbQuerySql(request);
        return TResponse.ok(ResponseCodeEnum.SUCCESS, list);
    }

    @ApiOperation(value = "发送消息汇总入口", notes = "sendMsgMaster")
    @PostMapping(value = "/send/msgMaster")
    public TResponse<WxPpWcfSendTextMsgResp> sendMsgMaster(@Validated @RequestBody String jsonString) {
        // todo
        return TResponse.ok(ResponseCodeEnum.SUCCESS);
    }

    @ApiOperation(value = "发送文本消息", notes = "sendTextMsg")
    @PostMapping(value = "/send/textMsg")
    public TResponse<WxPpWcfSendTextMsgResp> sendTextMsg(@Validated @RequestBody WxPpWcfSendTextMsgReq request) {
        WxPpWcfSendTextMsgResp resp = weChatDllService.sendTextMsg(request);
        return TResponse.ok(ResponseCodeEnum.SUCCESS, resp);
    }

    @ApiOperation(value = "发送富文本消息", notes = "sendRichTextMsg")
    @PostMapping(value = "/send/richTextMsg")
    public TResponse<WxPpWcfSendRichTextMsgResp> sendRichTextMsg(@Validated @RequestBody WxPpWcfSendRichTextMsgReq request) {
        WxPpWcfSendRichTextMsgResp resp = weChatDllService.sendRichTextMsg(request);
        return TResponse.ok(ResponseCodeEnum.SUCCESS, resp);
    }

    @ApiOperation(value = "发送XML消息", notes = "sendXmlMsg")
    @PostMapping(value = "/send/xmlMsg")
    public TResponse<WxPpWcfSendXmlMsgResp> sendXmlMsg(@Validated @RequestBody WxPpWcfSendXmlMsgReq request) {
        WxPpWcfSendXmlMsgResp resp = weChatDllService.sendXmlMsg(request);
        return TResponse.ok(ResponseCodeEnum.SUCCESS, resp);
    }

    @ApiOperation(value = "发送图片消息", notes = "sendImageMsg")
    @PostMapping(value = "/send/imageMsg")
    public TResponse<WxPpWcfSendImageMsgResp> sendImageMsg(@Validated @RequestBody WxPpWcfSendImageMsgReq request) {
        WxPpWcfSendImageMsgResp resp = weChatDllService.sendImageMsg(request);
        return TResponse.ok(ResponseCodeEnum.SUCCESS, resp);
    }

    @ApiOperation(value = "发送表情消息", notes = "sendEmojiMsg")
    @PostMapping(value = "/send/emojiMsg")
    public TResponse<WxPpWcfSendEmojiMsgResp> sendEmojiMsg(@Validated @RequestBody WxPpWcfSendEmojiMsgReq request) {
        WxPpWcfSendEmojiMsgResp resp = weChatDllService.sendEmojiMsg(request);
        return TResponse.ok(ResponseCodeEnum.SUCCESS, resp);
    }

    @ApiOperation(value = "发送文件消息", notes = "sendFileMsg")
    @PostMapping(value = "/send/fileMsg")
    public TResponse<WxPpWcfSendFileMsgResp> sendFileMsg(@Validated @RequestBody WxPpWcfSendFileMsgReq request) {
        WxPpWcfSendFileMsgResp resp = weChatDllService.sendFileMsg(request);
        return TResponse.ok(ResponseCodeEnum.SUCCESS, resp);
    }

    @ApiOperation(value = "拍一拍群友", notes = "patOnePat")
    @PostMapping(value = "/patOnePat")
    public TResponse<WxPpWcfSendPatOnePatMsgResp> patOnePat(@Validated @RequestBody WxPpWcfPatOnePatMsgReq request) {
        WxPpWcfSendPatOnePatMsgResp resp = weChatDllService.patOnePat(request);
        return TResponse.ok(ResponseCodeEnum.SUCCESS, resp);
    }

    // @ApiOperation(value = "获取语音消息", notes = "queryMsgTypeList")
    // @PostMapping(value = "/list/voiceMsg")
    // public TResponse<Object> queryVoiceMsg() {
    // return TResponse.ok(ResponseCodeEnum.SUCCESS, list);
    // }

    @ApiOperation(value = "撤回消息", notes = "revokeMsg")
    @PostMapping(value = "/revokeMsg")
    public TResponse<Object> revokeMsg(@Validated @RequestBody WxPpWcfRevokeMsgReq request) {
        weChatDllService.revokeMsg(request);
        return TResponse.ok(ResponseCodeEnum.SUCCESS);
    }

    // @ApiOperation(value = "转发消息", notes = "queryMsgTypeList")
    // @PostMapping(value = "/list/msgType")
    // public TResponse<Object> queryMsgTypeList() {
    // return TResponse.ok(ResponseCodeEnum.SUCCESS, list);
    // }
    //
    // @ApiOperation(value = "开启接收消息", notes = "queryMsgTypeList")
    // @PostMapping(value = "/list/msgType")
    // public TResponse<Object> queryMsgTypeList() {
    // return TResponse.ok(ResponseCodeEnum.SUCCESS, list);
    // }
    //
    // @ApiOperation(value = "关闭接收消息", notes = "queryMsgTypeList")
    // @PostMapping(value = "/list/msgType")
    // public TResponse<Object> queryMsgTypeList() {
    // return TResponse.ok(ResponseCodeEnum.SUCCESS, list);
    // }
    //

    @ApiOperation(value = "通过好友申请", notes = "passFriendApply")
    @PostMapping(value = "/passFriendApply")
    public TResponse<Object> passFriendApply(@Validated @RequestBody WxPpWcfPassFriendApplyReq request) {
        weChatDllService.passFriendApply(request);
        return TResponse.ok(ResponseCodeEnum.SUCCESS);
    }

    @ApiOperation(value = "添加群成员为微信好友", notes = "addFriendGroupMember")
    @PostMapping(value = "/addFriend/groupMember")
    public TResponse<Object> addFriendGroupMember(@Validated @RequestBody WxPpWcfAddFriendGroupMemberReq request) {
        weChatDllService.addFriendGroupMember(request);
        return TResponse.ok(ResponseCodeEnum.SUCCESS);
    }

    @ApiOperation(value = "查询群成员列表", notes = "queryGroupMemberList")
    @PostMapping(value = "/groupMember/list")
    public TResponse<List<WxPpWcfGroupMemberResp>> queryGroupMemberList(@Validated @RequestBody WxPpWcfGroupMemberReq request) {
        List<WxPpWcfGroupMemberResp> list = weChatDllService.queryGroupMemberList(request);
        return TResponse.ok(ResponseCodeEnum.SUCCESS, list);
    }

    @ApiOperation(value = "邀请群成员", notes = "inviteGroupMember")
    @PostMapping(value = "/groupMember/invite")
    public TResponse<Object> inviteGroupMember(@Validated @RequestBody WxPpWcfInviteGroupMemberReq request) {
        weChatDllService.inviteGroupMember(request);
        return TResponse.ok(ResponseCodeEnum.SUCCESS);
    }

    @ApiOperation(value = "删除群成员", notes = "deleteGroupMember")
    @PostMapping(value = "/groupMember/delete")
    public TResponse<Object> deleteGroupMember(@Validated @RequestBody WxPpWcfDeleteGroupMemberReq request) {
        weChatDllService.deleteGroupMember(request);
        return TResponse.ok(ResponseCodeEnum.SUCCESS);
    }

    @ApiOperation(value = "查询朋友圈", notes = "queryFriendCircle")
    @PostMapping(value = "/friendCircle")
    public TResponse<Object> queryFriendCircle() {
        weChatDllService.queryFriendCircle(0);
        return TResponse.ok(ResponseCodeEnum.SUCCESS);
    }

    @ApiOperation(value = "接收转账", notes = "receiveTransfer")
    @PostMapping(value = "/receiveTransfer")
    public TResponse<Object> receiveTransfer(@Validated @RequestBody WxPpWcfReceiveTransferReq request) {
        weChatDllService.receiveTransfer(request);
        return TResponse.ok(ResponseCodeEnum.SUCCESS);
    }

    // @ApiOperation(value = "下载图片、视频、文件", notes = "queryMsgTypeList")
    // @PostMapping(value = "/list/msgType")
    // public TResponse<Object> queryMsgTypeList() {
    // return TResponse.ok(ResponseCodeEnum.SUCCESS, list);
    // }
    //
    // @ApiOperation(value = "解密图片", notes = "queryMsgTypeList")
    // @PostMapping(value = "/list/msgType")
    // public TResponse<Object> queryMsgTypeList() {
    // return TResponse.ok(ResponseCodeEnum.SUCCESS, list);
    // }

}
