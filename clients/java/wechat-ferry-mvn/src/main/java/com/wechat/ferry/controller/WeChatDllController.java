package com.wechat.ferry.controller;

import java.util.List;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import com.wechat.ferry.entity.TResponse;
import com.wechat.ferry.entity.vo.request.WxPpDatabaseSqlReq;
import com.wechat.ferry.entity.vo.request.WxPpDatabaseTableReq;
import com.wechat.ferry.entity.vo.request.WxPpGroupMemberReq;
import com.wechat.ferry.entity.vo.request.WxPpSendCardMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpSendEmojiMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpSendFileMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpSendImageMsgReq;
import com.wechat.ferry.entity.vo.request.WxPpSendTextMsgReq;
import com.wechat.ferry.entity.vo.response.WxPpContactsResp;
import com.wechat.ferry.entity.vo.response.WxPpDatabaseRowResp;
import com.wechat.ferry.entity.vo.response.WxPpGroupMemberResp;
import com.wechat.ferry.entity.vo.response.WxPpLoginInfoResp;
import com.wechat.ferry.entity.vo.response.WxPpMsgTypeResp;
import com.wechat.ferry.entity.vo.response.WxPpSendCardMsgResp;
import com.wechat.ferry.entity.vo.response.WxPpSendEmojiMsgResp;
import com.wechat.ferry.entity.vo.response.WxPpSendFileMsgResp;
import com.wechat.ferry.entity.vo.response.WxPpSendImageMsgResp;
import com.wechat.ferry.entity.vo.response.WxPpSendTextMsgResp;
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

    @ApiOperation(value = "获取登录微信内部识别号UID", notes = "queryLoginWeChatUid")
    @PostMapping(value = "/loginWeChatUid")
    public TResponse<Object> queryLoginWeChatUid() {
        String weChatUid = weChatDllService.queryLoginWeChatUid();
        return TResponse.ok(ResponseCodeEnum.SUCCESS, weChatUid);
    }

    @ApiOperation(value = "获取登录微信信息", notes = "queryLoginWeChatInfo")
    @PostMapping(value = "/loginWeChatInfo")
    public TResponse<WxPpLoginInfoResp> queryLoginWeChatInfo() {
        WxPpLoginInfoResp resp = weChatDllService.queryLoginWeChatInfo();
        return TResponse.ok(ResponseCodeEnum.SUCCESS, resp);
    }

    @ApiOperation(value = "获取消息类型", notes = "queryMsgTypeList")
    @PostMapping(value = "/list/msgType")
    public TResponse<List<WxPpMsgTypeResp>> queryMsgTypeList() {
        List<WxPpMsgTypeResp> list = weChatDllService.queryMsgTypeList();
        return TResponse.ok(ResponseCodeEnum.SUCCESS, list);
    }

    @ApiOperation(value = "获取联系人", notes = "queryContactsList")
    @PostMapping(value = "/list/contacts")
    public TResponse<List<WxPpContactsResp>> queryContactsList() {
        List<WxPpContactsResp> list = weChatDllService.queryContactsList();
        return TResponse.ok(ResponseCodeEnum.SUCCESS, list);
    }

    @ApiOperation(value = "获取可查询数据库", notes = "queryDatabaseSql")
    @PostMapping(value = "/list/dbSql")
    public TResponse<List<WxPpDatabaseRowResp>> queryDatabaseSql(@Validated @RequestBody WxPpDatabaseSqlReq request) {
        List<WxPpDatabaseRowResp> list = weChatDllService.queryDatabaseSql(request);
        return TResponse.ok(ResponseCodeEnum.SUCCESS, list);
    }

    @ApiOperation(value = "获取数据库所有表名称", notes = "queryDatabaseAllTableName")
    @PostMapping(value = "/list/dbTableName")
    public TResponse<List<String>> queryDatabaseAllTableName() {
        List<String> list = weChatDllService.queryDatabaseAllTableName();
        return TResponse.ok(ResponseCodeEnum.SUCCESS, list);
    }

    @ApiOperation(value = "获取指定数据库中的表", notes = "queryDatabaseTable")
    @PostMapping(value = "/list/dbTable")
    public TResponse<List<String>> queryDatabaseTable(@Validated @RequestBody WxPpDatabaseTableReq request) {
        List<String> list = weChatDllService.queryDatabaseTable(request);
        return TResponse.ok(ResponseCodeEnum.SUCCESS, list);
    }

    // @ApiOperation(value = "获取语音消息", notes = "queryMsgTypeList")
    // @PostMapping(value = "/list/voiceMsg")
    // public TResponse<Object> queryVoiceMsg() {
    // return TResponse.ok(ResponseCodeEnum.SUCCESS, list);
    // }

    @ApiOperation(value = "查询群成员", notes = "queryGroupMember")
    @PostMapping(value = "/list/groupMember")
    public TResponse<List<WxPpGroupMemberResp>> queryGroupMember(@Validated @RequestBody WxPpGroupMemberReq request) {
        List<WxPpGroupMemberResp> list = weChatDllService.queryGroupMember(request);
        return TResponse.ok(ResponseCodeEnum.SUCCESS, list);
    }

    @ApiOperation(value = "发送消息汇总入口", notes = "sendMsgMaster")
    @PostMapping(value = "/send/msgMaster")
    public TResponse<WxPpSendTextMsgResp> sendMsgMaster(@Validated @RequestBody String jsonString) {

        return TResponse.ok(ResponseCodeEnum.SUCCESS);
    }

    @ApiOperation(value = "发送文本消息（可 @）", notes = "sendTextMsg")
    @PostMapping(value = "/send/textMsg")
    public TResponse<WxPpSendTextMsgResp> sendTextMsg(@Validated @RequestBody WxPpSendTextMsgReq request) {
        WxPpSendTextMsgResp resp = weChatDllService.sendTextMsg(request);
        return TResponse.ok(ResponseCodeEnum.SUCCESS, resp);
    }

    @ApiOperation(value = "发送图片消息", notes = "sendImageMsg")
    @PostMapping(value = "/send/imageMsg")
    public TResponse<WxPpSendImageMsgResp> sendImageMsg(@Validated @RequestBody WxPpSendImageMsgReq request) {
        WxPpSendImageMsgResp resp = weChatDllService.sendImageMsg(request);
        return TResponse.ok(ResponseCodeEnum.SUCCESS, resp);
    }

    @ApiOperation(value = "发送文件消息", notes = "sendFileMsg")
    @PostMapping(value = "/send/fileMsg")
    public TResponse<WxPpSendFileMsgResp> sendFileMsg(@Validated @RequestBody WxPpSendFileMsgReq request) {
        WxPpSendFileMsgResp resp = weChatDllService.sendFileMsg(request);
        return TResponse.ok(ResponseCodeEnum.SUCCESS, resp);
    }

    @ApiOperation(value = "发送卡片消息", notes = "sendCardMsg")
    @PostMapping(value = "/send/cardMsg")
    public TResponse<WxPpSendCardMsgResp> sendCardMsg(@Validated @RequestBody WxPpSendCardMsgReq request) {
        WxPpSendCardMsgResp resp = weChatDllService.sendCardMsg(request);
        return TResponse.ok(ResponseCodeEnum.SUCCESS, resp);
    }

    @ApiOperation(value = "发送表情消息", notes = "sendEmojiMsg")
    @PostMapping(value = "/send/emojiMsg")
    public TResponse<WxPpSendEmojiMsgResp> sendEmojiMsg(@Validated @RequestBody WxPpSendEmojiMsgReq request) {
        WxPpSendEmojiMsgResp resp = weChatDllService.sendEmojiMsg(request);
        return TResponse.ok(ResponseCodeEnum.SUCCESS, resp);
    }

    // @ApiOperation(value = "拍一拍群友", notes = "queryMsgTypeList")
    // @PostMapping(value = "/list/msgType")
    // public TResponse<Object> queryMsgTypeList() {
    // return TResponse.ok(ResponseCodeEnum.SUCCESS, list);
    // }
    //
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
    // @ApiOperation(value = "查询数据库", notes = "queryMsgTypeList")
    // @PostMapping(value = "/list/msgType")
    // public TResponse<Object> queryMsgTypeList() {
    // return TResponse.ok(ResponseCodeEnum.SUCCESS, list);
    // }
    //
    // @ApiOperation(value = "获取朋友圈消息", notes = "queryMsgTypeList")
    // @PostMapping(value = "/list/msgType")
    // public TResponse<Object> queryMsgTypeList() {
    // return TResponse.ok(ResponseCodeEnum.SUCCESS, list);
    // }
    //
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
    //
    // @ApiOperation(value = "添加群成员", notes = "queryMsgTypeList")
    // @PostMapping(value = "/list/msgType")
    // public TResponse<Object> queryMsgTypeList() {
    // return TResponse.ok(ResponseCodeEnum.SUCCESS, list);
    // }
    //
    // @ApiOperation(value = "删除群成员", notes = "queryMsgTypeList")
    // @PostMapping(value = "/list/msgType")
    // public TResponse<Object> queryMsgTypeList() {
    // return TResponse.ok(ResponseCodeEnum.SUCCESS, list);
    // }
    //
    // @ApiOperation(value = "邀请群成员", notes = "queryMsgTypeList")
    // @PostMapping(value = "/list/msgType")
    // public TResponse<Object> queryMsgTypeList() {
    // return TResponse.ok(ResponseCodeEnum.SUCCESS, list);
    // }

}
