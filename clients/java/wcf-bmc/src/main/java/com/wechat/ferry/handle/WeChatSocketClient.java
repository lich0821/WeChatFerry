package com.wechat.ferry.handle;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;
import java.util.function.Function;

import org.springframework.util.ObjectUtils;

import com.alibaba.fastjson2.JSONObject;
import com.google.protobuf.ByteString;
import com.sun.jna.Native;
import com.wechat.ferry.entity.dto.WxPpMsgDTO;
import com.wechat.ferry.entity.proto.Wcf;
import com.wechat.ferry.entity.proto.Wcf.DbQuery;
import com.wechat.ferry.entity.proto.Wcf.DbRow;
import com.wechat.ferry.entity.proto.Wcf.Functions;
import com.wechat.ferry.entity.proto.Wcf.Request;
import com.wechat.ferry.entity.proto.Wcf.Response;
import com.wechat.ferry.entity.proto.Wcf.WxMsg;
import com.wechat.ferry.exception.BizException;
import com.wechat.ferry.service.SDK;
import com.wechat.ferry.utils.HttpClientUtil;
import com.wechat.ferry.utils.XmlJsonConvertUtil;

import io.sisu.nng.Socket;
import io.sisu.nng.pair.Pair1Socket;
import lombok.extern.slf4j.Slf4j;

/**
 * 处理层-微信客户端
 *
 * @author Changhua
 * @date 2023-12-06 22:11
 */
@Slf4j
public class WeChatSocketClient {

    /**
     * 消息缓冲区大小，16M
     */
    private static final Integer BUFFER_SIZE = 16 * 1024 * 1024;

    /**
     * 默认IP
     */
    private static final String DEFAULT_HOST = "127.0.0.1";

    /**
     * 请求地址
     */
    private static final String CMD_URL = "tcp://%s:%s";

    private Socket cmdSocket = null;
    private Socket msgSocket = null;

    /**
     * 是否已启动接收消息
     */
    private boolean isReceivingMsg = false;

    /**
     * 是否为本地端口
     */
    private boolean isLocalHostPort = false;

    /**
     * 消息返回
     */
    private BlockingQueue<WxMsg> msgQ;

    private final String host;

    private final Integer port;

    public WeChatSocketClient(Integer port, String dllPath) {
        this(DEFAULT_HOST, port, false, dllPath);
    }

    public WeChatSocketClient(Integer port, boolean debug, String dllPath) {
        this(DEFAULT_HOST, port, debug, dllPath);
    }

    public WeChatSocketClient(String host, Integer port, boolean debug, String dllPath) {
        this.host = host;
        this.port = port;

        SDK INSTANCE = Native.load(dllPath, SDK.class);
        int status = INSTANCE.WxInitSDK(debug, port);
        if (status != 0) {
            log.error("启动 RPC 失败: {}", status);
            System.exit(-1);
        }
        connectRPC(String.format(CMD_URL, host, port), INSTANCE);
        if (DEFAULT_HOST.equals(host) || "localhost".equalsIgnoreCase(host)) {
            isLocalHostPort = true;
        }
    }

    public void connectRPC(String url, SDK INSTANCE) {
        try {
            cmdSocket = new Pair1Socket();
            cmdSocket.dial(url);
            while (!isLogin()) {
                // 直到登录成功
                waitMs(1000);
            }
        } catch (Exception e) {
            log.error("连接 RPC 失败: ", e);
            System.exit(-1);
        }
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            log.info("关闭...");
            diableRecvMsg();
            if (isLocalHostPort) {
                INSTANCE.WxDestroySDK();
            }
        }));
    }

    public Response sendCmd(Request req) {
        try {
            // 不知道之前设置20S有啥特殊情况？？？这里设置超时时间 5s --> 参考Python版本
            // 防止无响应的时候线程一直阻塞--ReceiveTimeout
            // modify by wmz 2025-05-03
            cmdSocket.setSendTimeout(5000);
            cmdSocket.setReceiveTimeout(5000);
            
            ByteBuffer bb = ByteBuffer.wrap(req.toByteArray());
            cmdSocket.send(bb);
            ByteBuffer ret = ByteBuffer.allocate(BUFFER_SIZE);
            long size = cmdSocket.receive(ret, true);
            return Response.parseFrom(Arrays.copyOfRange(ret.array(), 0, (int)size));
        } catch (Exception e) {
            if ("Timed out".equals(e.getMessage())) {
                log.error("请求超时: ", e);
                throw new BizException("请求超时:1.接口耗时太长，2.服务与客户端失去联系，请重启本服务！详细异常信息：" + e.getMessage());
            } else {
                log.error("命令调用失败: ", e);
                throw new BizException("命令调用失败:" + e.getMessage());
            }
        }
    }

    private void listenMsg(String url) {
        try {
            msgSocket = new Pair1Socket();
            msgSocket.dial(url);
            // 设置接收 5 秒超时
            msgSocket.setReceiveTimeout(5000);
        } catch (Exception e) {
            log.error("创建消息 RPC 失败", e);
            return;
        }
        ByteBuffer bb = ByteBuffer.allocate(BUFFER_SIZE);
        while (isReceivingMsg) {
            try {
                long size = msgSocket.receive(bb, true);
                WxMsg wxMsg = Response.parseFrom(Arrays.copyOfRange(bb.array(), 0, (int)size)).getWxmsg();
                msgQ.put(wxMsg);
            } catch (Exception e) {
                // 多半是超时，忽略吧
            }
        }
        try {
            msgSocket.close();
        } catch (Exception e) {
            log.error("关闭连接失败", e);
        }
    }

    /**
     * 获取登录二维码，已经登录则返回空字符串
     *
     * @return 是否登录结果
     */
    public Response getQrcode() {
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_REFRESH_QRCODE_VALUE).build();
        Response rsp = sendCmd(req);
        if (rsp != null) {
            return rsp;
        }
        return null;
    }

    /**
     * 是否已经登录
     * 当前微信客户端是否登录微信号
     *
     * @return 是否登录结果
     */
    public boolean isLogin() {
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_IS_LOGIN_VALUE).build();
        Response rsp = sendCmd(req);
        if (rsp != null) {
            return rsp.getStatus() == 1;
        }
        return false;
    }

    /**
     * 获取登录账户的wxid
     *
     * @return 自己的微信号
     */
    public String getSelfWxId() {
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_GET_SELF_WXID_VALUE).build();
        Response rsp = sendCmd(req);
        if (rsp != null) {
            return rsp.getStr();
        }
        return "";
    }

    /**
     * 获取所有消息类型
     *
     * @return 消息类型集合
     */
    public Map<Integer, String> getMsgTypes() {
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_GET_MSG_TYPES_VALUE).build();
        Response rsp = sendCmd(req);
        if (rsp != null) {
            return rsp.getTypes().getTypesMap();
        }
        return Wcf.MsgTypes.newBuilder().build().getTypesMap();
    }

    /**
     * 获取完整通讯录
     * "fmessage": "朋友推荐消息",
     * "medianote": "语音记事本",
     * "floatbottle": "漂流瓶",
     * "filehelper": "文件传输助手",
     * "newsapp": "新闻",
     *
     * @return 联系人列表
     */
    public List<Wcf.RpcContact> getContacts() {
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_GET_CONTACTS_VALUE).build();
        Response rsp = sendCmd(req);
        if (rsp != null) {
            return rsp.getContacts().getContactsList();
        }
        return Wcf.RpcContacts.newBuilder().build().getContactsList();
    }

    /**
     * 获取所有数据库
     *
     * @return
     */
    public List<String> getDbNames() {
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_GET_DB_NAMES_VALUE).build();
        Response rsp = sendCmd(req);
        if (rsp != null) {
            return rsp.getDbs().getNamesList();
        }
        return Wcf.DbNames.newBuilder().build().getNamesList();
    }

    /**
     * 获取 db 中所有表
     *
     * @param db 数据库名
     * @return 该数据库下的所有表名及对应建表语句
     */
    public Map<String, String> getDbTables(String db) {
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_GET_DB_TABLES_VALUE).setStr(db).build();
        Response rsp = sendCmd(req);
        Map<String, String> tables = new LinkedHashMap<>();
        if (rsp != null) {
            for (Wcf.DbTable tbl : rsp.getTables().getTablesList()) {
                tables.put(tbl.getName(), tbl.getSql());
            }
        }
        return tables;
    }

    /**
     * 获取登录账号个人信息
     *
     * @return 个人信息
     */
    public Wcf.UserInfo getUserInfo() {
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_GET_USER_INFO_VALUE).build();
        Response rsp = sendCmd(req);
        if (rsp != null) {
            return rsp.getUi();
        }
        return null;
    }

    /**
     * 获取语音消息并转成 MP3
     *
     * @param id 语音消息 id
     * @param dir MP3 保存目录（目录不存在会出错）
     * @param timeout MP3 超时时间（秒）
     * @return 成功返回存储路径；空字符串为失败，原因见日志。
     */
    public String getAudioMsg(Integer id, String dir, Integer timeout) {
        Wcf.AudioMsg audioMsg = Wcf.AudioMsg.newBuilder().setId(id).setDir(dir).build();
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_GET_AUDIO_MSG_VALUE).setAm(audioMsg).build();
        Response rsp = sendCmd(req);
        if (rsp != null) {
            return rsp.getStr();
        }
        return null;
    }

    /**
     * 发送文本消息
     *
     * @param msg: 消息内容（如果是 @ 消息则需要有跟 @ 的人数量相同的 @）
     * @param receiver: 消息接收人，私聊为 wxid（wxid_xxxxxxxxxxxxxx），群聊为
     *            roomid（xxxxxxxxxx@chatroom）
     * @param aters: 群聊时要 @ 的人（私聊时为空字符串），多个用逗号分隔。@所有人 用
     *            notify@all（必须是群主或者管理员才有权限）
     * @return int 0 为成功，其他失败
     * @author Changhua
     * @example sendText(" Hello @ 某人1 @ 某人2 ", " xxxxxxxx @ chatroom ",
     *          "wxid_xxxxxxxxxxxxx1,wxid_xxxxxxxxxxxxx2");
     **/
    public int sendText(String msg, String receiver, String aters) {
        Wcf.TextMsg textMsg = Wcf.TextMsg.newBuilder().setMsg(msg).setReceiver(receiver).setAters(aters).build();
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_SEND_TXT_VALUE).setTxt(textMsg).build();
        log.debug("sendText: {}", bytesToHex(req.toByteArray()));
        Response rsp = sendCmd(req);
        int ret = -1;
        if (rsp != null) {
            ret = rsp.getStatus();
        }
        return ret;
    }

    /**
     * 发送图片消息
     *
     * @param path 图片地址,如：`C:/Projs/WeChatRobot/TEQuant.jpeg` 或
     *            `https://raw.githubusercontent.com/lich0821/WeChatFerry/master/assets/TEQuant.jpg`
     * @param receiver 消息接收人，wxid 或者 roomid
     * @return 发送结果状态码 0 为成功，其他失败
     */
    public int sendImage(String path, String receiver) {
        Wcf.PathMsg pathMsg = Wcf.PathMsg.newBuilder().setPath(path).setReceiver(receiver).build();
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_SEND_IMG_VALUE).setFile(pathMsg).build();
        log.debug("sendImage: {}", bytesToHex(req.toByteArray()));
        Response rsp = sendCmd(req);
        int ret = -1;
        if (rsp != null) {
            ret = rsp.getStatus();
        }
        return ret;
    }

    /**
     * 发送文件消息
     *
     * @param path 本地文件路径，如：`C:/Projs/WeChatRobot/README.MD` 或
     *            `https://raw.githubusercontent.com/lich0821/WeChatFerry/master/README.MD`
     * @param receiver 消息接收人，wxid 或者 roomid
     * @return 发送结果状态码 0 为成功，其他失败
     */
    public int sendFile(String path, String receiver) {
        Wcf.PathMsg pathMsg = Wcf.PathMsg.newBuilder().setPath(path).setReceiver(receiver).build();
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_SEND_FILE_VALUE).setFile(pathMsg).build();
        log.debug("sendFile: {}", bytesToHex(req.toByteArray()));
        Response rsp = sendCmd(req);
        int ret = -1;
        if (rsp != null) {
            ret = rsp.getStatus();
        }
        return ret;
    }

    /**
     * 发送Xml消息
     *
     * @param receiver 消息接收人，wxid 或者 roomid
     * @param xml xml内容
     * @param path 封面图片路径
     * @param type xml 类型，如：0x21 为小程序
     * @return 发送结果状态码 0 为成功，其他失败
     */
    public int sendXml(String receiver, String xml, String path, int type) {
        Wcf.XmlMsg xmlMsg = Wcf.XmlMsg.newBuilder().setContent(xml).setReceiver(receiver).setPath(path).setType(type).build();
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_SEND_XML_VALUE).setXml(xmlMsg).build();
        log.debug("sendXml: {}", bytesToHex(req.toByteArray()));
        Response rsp = sendCmd(req);
        int ret = -1;
        if (rsp != null) {
            ret = rsp.getStatus();
        }
        return ret;
    }

    /**
     * 发送表情消息
     *
     * @param path 本地表情路径，如：`C:/Projs/WeChatRobot/emo.gif`
     * @param receiver 消息接收人，wxid 或者 roomid
     * @return 发送结果状态码 0 为成功，其他失败
     */
    public int sendEmotion(String path, String receiver) {
        Wcf.PathMsg pathMsg = Wcf.PathMsg.newBuilder().setPath(path).setReceiver(receiver).build();
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_SEND_EMOTION_VALUE).setFile(pathMsg).build();
        log.debug("sendEmotion: {}", bytesToHex(req.toByteArray()));
        Response rsp = sendCmd(req);
        int ret = -1;
        if (rsp != null) {
            ret = rsp.getStatus();
        }
        return ret;
    }

    /**
     * 发送富文本消息
     *
     * @param name 左下显示的名字
     * @param account 填公众号 id 可以显示对应的头像（gh_ 开头的）
     * @param title 标题，最多两行
     * @param digest 摘要，三行
     * @param url 点击后跳转的链接
     * @param thumbUrl 缩略图的链接
     * @param receiver 接收人, wxid 或者 roomid
     * @return 发送结果状态码 0 为成功，其他失败
     *
     * @example 卡片样式：
     *          |-------------------------------------|
     *          |title, 最长两行
     *          |(长标题, 标题短的话这行没有)
     *          |digest, 最多三行，会占位 |--------|
     *          |digest, 最多三行，会占位 |thumbUrl|
     *          |digest, 最多三行，会占位 |--------|
     *          |(account logo) name
     *          |-------------------------------------|
     */
    public int sendRichText(String name, String account, String title, String digest, String url, String thumbUrl, String receiver) {
        Wcf.RichText richTextMsg = Wcf.RichText.newBuilder().setName(name).setAccount(account).setTitle(title).setDigest(digest).setUrl(url)
            .setThumburl(thumbUrl).setReceiver(receiver).build();
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_SEND_RICH_TXT_VALUE).setRt(richTextMsg).build();
        log.debug("sendRichText: {}", bytesToHex(req.toByteArray()));
        Response rsp = sendCmd(req);
        int ret = -1;
        if (rsp != null) {
            ret = rsp.getStatus();
        }
        return ret;
    }

    /**
     * 拍一拍群友
     *
     * @param roomid 群 id
     * @param wxid 要拍的群友的 wxid
     * @return 发送结果状态码 0 为成功，其他失败
     */
    public int sendPatMsg(String roomid, String wxid) {
        Wcf.PatMsg patMsg = Wcf.PatMsg.newBuilder().setRoomid(roomid).setWxid(wxid).build();
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_SEND_PAT_MSG_VALUE).setPm(patMsg).build();
        log.debug("sendPatMsg: {}", bytesToHex(req.toByteArray()));
        Response rsp = sendCmd(req);
        int ret = -1;
        if (rsp != null) {
            // 接口中 1 为成功，其他失败
            ret = rsp.getStatus();
            if (ret == 1) {
                // 转为通用值
                ret = 0;
            }
        }
        return ret;
    }

    /**
     * 转发消息。可以转发文本、图片、表情、甚至各种 XML；
     * 语音也行，不过效果嘛，自己验证吧。
     *
     * @param id 待转发消息的 id
     * @param receiver 消息接收者，wxid 或者 roomid
     * @return 结果状态码 0 为成功，其他失败
     */
    public int forwardMsg(Integer id, String receiver) {
        Wcf.ForwardMsg forwardMsg = Wcf.ForwardMsg.newBuilder().setId(id).setReceiver(receiver).build();
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_FORWARD_MSG_VALUE).setFm(forwardMsg).build();
        log.debug("forwardMsg: {}", bytesToHex(req.toByteArray()));
        Response rsp = sendCmd(req);
        int ret = -1;
        if (rsp != null) {
            // 接口中 1 为成功，其他失败
            ret = rsp.getStatus();
            if (ret == 1) {
                // 转为通用值
                ret = 0;
            }
        }
        return ret;
    }

    /**
     * 获取sql执行结果
     *
     * @param db 数据库名,要查询的数据库
     * @param sql 执行的sql语句
     * @return 数据记录列表
     */
    public List<DbRow> querySql(String db, String sql) {
        DbQuery dbQuery = DbQuery.newBuilder().setSql(sql).setDb(db).build();
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_EXEC_DB_QUERY_VALUE).setQuery(dbQuery).build();
        Response rsp = sendCmd(req);
        if (rsp != null) {
            return rsp.getRows().getRowsList();
        }
        return null;
    }

    /**
     * 通过好友申请
     *
     * @param v3 加密用户名 (好友申请消息里 v3 开头的字符串) xml.attrib["encryptusername"]
     * @param v4 Ticket (好友申请消息里 v4 开头的字符串) xml.attrib["ticket"]
     * @param scene 申请方式 (好友申请消息里的 scene); 为了兼容旧接口，默认为扫码添加 (30)
     * @return 结果状态码 0 为成功，其他失败
     */
    public int acceptNewFriend(String v3, String v4, Integer scene) {
        if (ObjectUtils.isEmpty(scene)) {
            scene = 30;
        }
        int ret = -1;
        Wcf.Verification verification = Wcf.Verification.newBuilder().setV3(v3).setV4(v4).setScene(scene).build();
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_ACCEPT_FRIEND_VALUE).setV(verification).build();
        Response rsp = sendCmd(req);
        if (rsp != null) {
            // 接口中 1 为成功，其他失败
            ret = rsp.getStatus();
            if (ret == 1) {
                // 转为通用值
                ret = 0;
            }
        }
        return ret;
    }

    /**
     * 接收转账
     *
     * @param wxid 转账消息里的发送人 wxid
     * @param transferid 转账消息里的 transferid
     * @param transactionid 转账消息里的 transactionid
     * @return 结果状态码 0 为成功，其他失败
     */
    public int receiveTransfer(String wxid, String transferid, String transactionid) {
        int ret = -1;
        Wcf.Transfer transfer = Wcf.Transfer.newBuilder().setWxid(wxid).setTfid(transferid).setTaid(transactionid).build();
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_RECV_TRANSFER_VALUE).setTf(transfer).build();
        Response rsp = sendCmd(req);
        if (rsp != null) {
            // 接口中 1 为成功，其他失败
            ret = rsp.getStatus();
            if (ret == 1) {
                // 转为通用值
                ret = 0;
            }
        }
        return ret;
    }

    /**
     * 刷新朋友圈
     *
     * @param id 开始 id，0 为最新页
     * @return 结果状态码 0 为成功，其他失败
     */
    public int refreshPyq(Integer id) {
        int ret = -1;
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_REFRESH_PYQ_VALUE).setUi64(id).build();
        Response rsp = sendCmd(req);
        if (rsp != null) {
            // 接口中 1 为成功，其他失败
            ret = rsp.getStatus();
            if (ret == 1) {
                // 转为通用值
                ret = 0;
            }
        }
        return ret;
    }

    /**
     * 下载附件（图片、视频、文件）。这方法别直接调用，下载图片使用 `download_image`。
     *
     * @param id 消息中 id
     * @param thumb 消息中的 thumb
     * @param extra 消息中的 extra
     * @return 结果状态码 0 为成功，其他失败
     */
    public int downloadAttach(Integer id, String thumb, String extra) {
        int ret = -1;
        Wcf.AttachMsg attachMsg = Wcf.AttachMsg.newBuilder().setId(id).setThumb(thumb).setExtra(extra).build();
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_DOWNLOAD_ATTACH_VALUE).setAtt(attachMsg).build();
        Response rsp = sendCmd(req);
        if (rsp != null) {
            // 接口中 1 为成功，其他失败
            ret = rsp.getStatus();
            if (ret == 1) {
                // 转为通用值
                ret = 0;
            }
        }
        return ret;
    }

    /**
     * 通过 wxid 查询微信号昵称等信息
     *
     * @param wxid 联系人 wxid
     * @return 结果信息
     */
    public Wcf.RpcContact getInfoByWxId(String wxid) {
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_GET_CONTACT_INFO_VALUE).setStr(wxid).build();
        Response rsp = sendCmd(req);
        if (rsp != null && !ObjectUtils.isEmpty(rsp.getContacts())) {
            return rsp.getContacts().getContacts(0);
        }
        return null;
    }

    /**
     * 撤回消息
     *
     * @param id 待撤回消息的 id
     * @return 结果状态码 0 为成功，其他失败
     */
    public int revokeMsg(Integer id) {
        int ret = -1;
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_REVOKE_MSG_VALUE).setUi64(id).build();
        Response rsp = sendCmd(req);
        if (rsp != null) {
            // 接口中 1 为成功，其他失败
            ret = rsp.getStatus();
            if (ret == 1) {
                // 转为通用值
                ret = 0;
            }
        }
        return ret;
    }

    /**
     * 解密图片。这方法别直接调用，下载图片使用 `download_image`。
     *
     * @param src 加密的图片路径
     * @param dir 保存图片的目录
     * @return 解密图片的保存路径
     * @see  WeChatDllServiceImpl decryptImage
     */
    @Deprecated
    public String decryptImage(String src, String dir) {
        Wcf.DecPath build = Wcf.DecPath.newBuilder().setSrc(src).setDst(dir).build();
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_DECRYPT_IMAGE_VALUE).setDec(build).build();
        Response rsp = sendCmd(req);
        if (rsp != null) {
            return rsp.getStr();
        }
        return "";
    }

    /**
     * 获取 OCR 结果。鸡肋，需要图片能自动下载；通过下载接口下载的图片无法识别。
     *
     * @param extra 待识别的图片路径，消息里的 extra
     * @return OCR 结果
     */
    public Wcf.OcrMsg getOcrResult(String extra, Integer timeout) {
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_EXEC_OCR_VALUE).setStr(extra).build();
        Response rsp = sendCmd(req);
        if (rsp != null) {
            return rsp.getOcr();
        }
        return null;
    }

    /**
     * 添加群成员
     *
     * @param roomid 待加群的 id
     * @param wxids 要加到群里的 wxid，多个用逗号分隔
     * @return 结果状态码 0 为成功，其他失败
     */
    public int addChatroomMembers(String roomid, String wxids) {
        int ret = -1;
        Wcf.MemberMgmt memberMgmt = Wcf.MemberMgmt.newBuilder().setRoomid(roomid).setWxids(wxids).build();
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_ADD_ROOM_MEMBERS_VALUE).setM(memberMgmt).build();
        Response rsp = sendCmd(req);
        if (rsp != null) {
            // 接口中 1 为成功，其他失败
            ret = rsp.getStatus();
            if (ret == 1) {
                // 转为通用值
                ret = 0;
            }
        }
        return ret;
    }

    /**
     * 删除群成员
     *
     * @param roomid 群的 id
     * @param wxids 要删除成员的 wxid，多个用逗号分隔
     * @return 结果状态码 0 为成功，其他失败
     */
    public int delChatroomMembers(String roomid, String wxids) {
        int ret = -1;
        Wcf.MemberMgmt memberMgmt = Wcf.MemberMgmt.newBuilder().setRoomid(roomid).setWxids(wxids).build();
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_DEL_ROOM_MEMBERS_VALUE).setM(memberMgmt).build();
        Response rsp = sendCmd(req);
        if (rsp != null) {
            // 接口中 1 为成功，其他失败
            ret = rsp.getStatus();
            if (ret == 1) {
                // 转为通用值
                ret = 0;
            }
        }
        return ret;
    }

    /**
     * 邀请群成员
     *
     * @param roomid 群的 id
     * @param wxids 要邀请成员的 wxid, 多个用逗号`,`分隔
     * @return 结果状态码 0 为成功，其他失败
     */
    public int inviteChatroomMembers(String roomid, String wxids) {
        int ret = -1;
        Wcf.MemberMgmt memberMgmt = Wcf.MemberMgmt.newBuilder().setRoomid(roomid).setWxids(wxids).build();
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_INV_ROOM_MEMBERS_VALUE).setM(memberMgmt).build();
        Response rsp = sendCmd(req);
        if (rsp != null) {
            // 接口中 1 为成功，其他失败
            ret = rsp.getStatus();
            if (ret == 1) {
                // 转为通用值
                ret = 0;
            }
        }
        return ret;
    }

    /**
     * 是否已启动接收消息功能
     */
    public boolean getIsReceivingMsg() {
        return isReceivingMsg;
    }

    public WxMsg getMsg() {
        try {
            return msgQ.take();
        } catch (Exception e) {
            // TODO: handle exception
            return null;
        }
    }

    /**
     * 判断是否是艾特自己的消息
     *
     * @param wxMsgXml XML消息
     * @param wxMsgContent 消息内容
     * @param selfWxId 自己的微信id
     * @return 是否
     */
    public boolean isAtMeMsg(String wxMsgXml, String wxMsgContent, String selfWxId) {
        String format = String.format("<atuserlist><![CDATA[%s]]></atuserlist>", selfWxId);
        boolean isAtAll = wxMsgContent.startsWith("@所有人") || wxMsgContent.startsWith("@all");
        if (wxMsgXml.contains(format) && !isAtAll) {
            return true;
        }
        return false;
    }

    /**
     * 允许接收消息
     */
    public void enableRecvMsg(int qSize) {
        if (isReceivingMsg) {
            return;
        }
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_ENABLE_RECV_TXT_VALUE).build();
        Response rsp = sendCmd(req);
        if (rsp == null) {
            log.error("启动消息接收失败");
            isReceivingMsg = false;
            return;
        }

        isReceivingMsg = true;
        msgQ = new ArrayBlockingQueue<>(qSize);
        String msgUrl = "tcp://" + this.host + ":" + (this.port + 1);
        Thread thread = new Thread(() -> listenMsg(msgUrl));
        thread.start();
    }

    /**
     * 停止接收消息
     */
    public int diableRecvMsg() {
        if (!isReceivingMsg) {
            return 1;
        }
        int ret = -1;
        Request req = Request.newBuilder().setFuncValue(Functions.FUNC_DISABLE_RECV_TXT_VALUE).build();
        Response rsp = sendCmd(req);
        if (rsp != null) {
            ret = rsp.getStatus();
            if (ret == 0) {
                isReceivingMsg = false;
            }
        }
        return ret;
    }

    public void waitMs(int ms) {
        try {
            Thread.sleep(ms);
        } catch (InterruptedException ex) {
            Thread.currentThread().interrupt();
        }
    }

    public void printWxMsg(WxMsg msg) {
        WxPpMsgDTO dto = new WxPpMsgDTO();
        dto.setIsSelf(msg.getIsSelf());
        dto.setIsGroup(msg.getIsGroup());
        dto.setId(msg.getId());
        dto.setType(msg.getType());
        dto.setTs(msg.getTs());
        dto.setRoomId(msg.getRoomid());
        dto.setContent(msg.getContent());
        dto.setSender(msg.getSender());
        dto.setSign(msg.getSign());
        dto.setThumb(msg.getThumb());
        dto.setExtra(msg.getExtra());
        dto.setXml(msg.getXml().replace("\n", "").replace("\t", ""));

        String jsonString = JSONObject.toJSONString(dto);
        log.info("收到消息: {}", jsonString);
    }

    public String bytesToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder();
        for (byte b : bytes) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }

    public void keepRunning() {
        while (true) {
            waitMs(1000);
        }
    }

    /**
     * 本机回调解析消息
     *
     * @param msg 消息内容
     * @param url 回调地址
     *
     * @author chandler
     * @date 2024-10-05 12:50
     */
    public void localCallbackAnalyzeMsg(WxMsg msg, String url) {
        String xml = msg.getXml();
        xml = xml.replaceAll(">[\\s\\p{Zs}]*<", "><");
        String content = msg.getContent();
        content = content.replaceAll(">[\\s\\p{Zs}]*<", "><");

        WxPpMsgDTO dto = new WxPpMsgDTO();
        dto.setIsSelf(msg.getIsSelf());
        dto.setIsGroup(msg.getIsGroup());
        dto.setId(msg.getId());
        dto.setType(msg.getType());
        dto.setTs(msg.getTs());
        dto.setRoomId(msg.getRoomid());
        dto.setSender(msg.getSender());
        dto.setSign(msg.getSign());
        dto.setThumb(msg.getThumb());
        dto.setExtra(msg.getExtra());
        dto.setXml(xml);
        // 根据消息类型判断 引用-49
        if (!ObjectUtils.isEmpty(msg.getContent()) && "49".equals("" + msg.getType())) {
            try {
                dto.setQuoteContent(content);
                JSONObject json = XmlJsonConvertUtil.xml2Json(content);
                // 获取第一层级的JSONObject
                JSONObject level1 = json.getJSONObject("msg");
                if (!ObjectUtils.isEmpty(level1)) {
                    // 获取第二层级的JSONObject
                    JSONObject level2 = level1.getJSONObject("appmsg");
                    if (!ObjectUtils.isEmpty(level2)) {
                        // 获取field字段的值
                        String fieldValue = level2.getString("title");
                        dto.setContent(fieldValue);
                    }
                }
                dto.setJsonContent(json);
            } catch (Exception e) {
                log.error("XML提取报错：", e);
                // 报错就使用原值
                dto.setContent(content);
            }
        } else {
            dto.setContent(content);
        }

        String jsonString = JSONObject.toJSONString(dto);
        try {
            String responseStr = HttpClientUtil.doPostJson(url, jsonString);
            if (!JSONObject.parseObject(responseStr).getString("code").equals("200")) {
                log.error("本机消息回调失败！-URL：{}", url);
            }
        } catch (Exception e) {
            log.error("本机消息回调接口报错：", e);
        }
    }

    /**
     * 获取SQL类型
     *
     * @param type 转换类型
     * @return 函数
     *
     * @author chandler
     * @date 2024-10-05 12:54
     */
    public Function<byte[], Object> getSqlType(int type) {
        Map<Integer, Function<byte[], Object>> sqlTypeMap = new HashMap<>();
        // 初始化SQL_TYPES 根据类型执行不同的Func
        sqlTypeMap.put(1, bytes -> new String(bytes, StandardCharsets.UTF_8));
        sqlTypeMap.put(2, bytes -> ByteBuffer.wrap(bytes).getFloat());
        sqlTypeMap.put(3, bytes -> new String(bytes, StandardCharsets.UTF_8));
        sqlTypeMap.put(4, bytes -> bytes);
        sqlTypeMap.put(5, bytes -> null);
        return sqlTypeMap.get(type);
    }

    /**
     * SQL转换类型
     *
     * @param type 转换类型
     * @param content 待转换内容
     *
     * @author chandler
     * @date 2024-10-05 12:54
     */
    public Object convertSqlVal(int type, ByteString content) {
        // 根据每一列的类型转换
        Function<byte[], Object> converter = getSqlType(type);
        if (converter != null) {
            return converter.apply(content.toByteArray());
        } else {
            log.warn("[SQL转换类型]-未知的SQL类型: {}", type);
            return content.toByteArray();
        }
    }

}
