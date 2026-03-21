package com.iamteer;

import com.iamteer.Wcf.*;
import io.sisu.nng.Socket;
import io.sisu.nng.pair.Pair1Socket;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

public class Client {

  private static final Logger logger = LoggerFactory.getLogger(Client.class);
  private static final int BUFFER_SIZE = 16 * 1024 * 1024; // 16M
  private Socket cmdSocket = null;
  private Socket msgSocket = null;
  private static String DEFAULT_HOST = "127.0.0.1";
  private static int PORT = 10086;
  private static String CMDURL = "tcp://%s:%s";
  private boolean isReceivingMsg = false;
  private boolean isLocalHostPort = false;
  private BlockingQueue<WxMsg> msgQ;

  private String host;
  private int port;

  public Client() {
    this(DEFAULT_HOST, PORT, false);
  }

  public Client(String host, int port, boolean debug) {
    this.host = host;
    this.port = port;
    int status = SDK.INSTANCE.WxInitSDK(debug, 10086);
    if (status != 0) {
      logger.error("启动 RPC 失败: {}", status);
      System.exit(-1);
    }
    connectRPC(String.format(CMDURL, host, port));
    if (DEFAULT_HOST.equals(host) || "localhost".equalsIgnoreCase(host)) {
      isLocalHostPort = true;
    }
  }


  public void connectRPC(String url) {
    try {
      cmdSocket = new Pair1Socket();
      cmdSocket.dial(url);
      logger.info("请点击登录微信");
      while (!isLogin()) { // 直到登录成功
        waitMs(1000);
      }
    } catch (Exception e) {
      logger.error("连接 RPC 失败: ", e);
      System.exit(-1);
    }
    Runtime.getRuntime().addShutdownHook(new Thread(() -> {
      logger.info("关闭...");
      diableRecvMsg();
      if (isLocalHostPort) {
        SDK.INSTANCE.WxDestroySDK();
      }
    }));
  }

  private Response sendCmd(Request req) {
    try {
      ByteBuffer bb = ByteBuffer.wrap(req.toByteArray());
      cmdSocket.send(bb);
      ByteBuffer ret = ByteBuffer.allocate(BUFFER_SIZE);
      long size = cmdSocket.receive(ret, true);
      return Response.parseFrom(Arrays.copyOfRange(ret.array(), 0, (int) size));
    } catch (Exception e) {
      logger.error("命令调用失败: ", e);
      return null;
    }
  }

  /**
   * 当前微信客户端是否登录微信号
   *
   * @return
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
   * 获得微信客户端登录的微信ID
   *
   * @return
   */
  public String getSelfWxid() {
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
   * @return
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
   * 获取所有联系人
   * "fmessage": "朋友推荐消息",
   * "medianote": "语音记事本",
   * "floatbottle": "漂流瓶",
   * "filehelper": "文件传输助手",
   * "newsapp": "新闻",
   *
   * @return
   */
  public List<RpcContact> getContacts() {
    Request req = Request.newBuilder().setFuncValue(Functions.FUNC_GET_CONTACTS_VALUE).build();
    Response rsp = sendCmd(req);
    if (rsp != null) {
      return rsp.getContacts().getContactsList();
    }

    return Wcf.RpcContacts.newBuilder().build().getContactsList();
  }

  /**
   * 获取sql执行结果
   *
   * @param db  数据库名
   * @param sql 执行的sql语句
   * @return
   */
  public List<DbRow> querySql(String db, String sql) {
    DbQuery dbQuery = DbQuery.newBuilder().setSql(sql).setDb(db).build();
    Request req = Request.newBuilder().setFuncValue(Functions.FUNC_EXEC_DB_QUERY_VALUE)
        .setQuery(dbQuery).build();
    Response rsp = sendCmd(req);
    if (rsp != null) {
      return rsp.getRows().getRowsList();
    }
    return null;
  }

  /**
   * 获取所有数据库名
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
   * 获取指定数据库中的所有表
   *
   * @param db
   * @return
   */
  public Map<String, String> getDbTables(String db) {
    Request req = Request.newBuilder().setFuncValue(Functions.FUNC_GET_DB_TABLES_VALUE).setStr(db)
        .build();
    Response rsp = sendCmd(req);
    Map<String, String> tables = new HashMap<>();
    if (rsp != null) {
      for (DbTable tbl : rsp.getTables().getTablesList()) {
        tables.put(tbl.getName(), tbl.getSql());
      }
    }

    return tables;
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
   **/
  public int sendText(String msg, String receiver, String aters) {
    Wcf.TextMsg textMsg = Wcf.TextMsg.newBuilder().setMsg(msg).setReceiver(receiver).setAters(aters)
        .build();
    Request req = Request.newBuilder().setFuncValue(Functions.FUNC_SEND_TXT_VALUE).setTxt(textMsg)
        .build();
    logger.debug("sendText: {}", bytesToHex(req.toByteArray()));
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
   * @param path     图片地址
   * @param receiver 接收者微信id
   * @return 发送结果状态码
   */
  public int sendImage(String path, String receiver) {
    Wcf.PathMsg pathMsg = Wcf.PathMsg.newBuilder().setPath(path).setReceiver(receiver).build();
    Request req = Request.newBuilder().setFuncValue(Functions.FUNC_SEND_IMG_VALUE).setFile(pathMsg)
        .build();
    logger.debug("sendImage: {}", bytesToHex(req.toByteArray()));
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
   * @param path     文件地址
   * @param receiver 接收者微信id
   * @return 发送结果状态码
   */
  public int sendFile(String path, String receiver) {
    Wcf.PathMsg pathMsg = Wcf.PathMsg.newBuilder().setPath(path).setReceiver(receiver).build();
    Request req = Request.newBuilder().setFuncValue(Functions.FUNC_SEND_FILE_VALUE).setFile(pathMsg)
        .build();
    logger.debug("sendFile: {}", bytesToHex(req.toByteArray()));
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
   * @param receiver 接收者微信id
   * @param xml      xml内容
   * @param path
   * @param type
   * @return 发送结果状态码
   */
  public int sendXml(String receiver, String xml, String path, int type) {
    Wcf.XmlMsg xmlMsg = Wcf.XmlMsg.newBuilder().setContent(xml).setReceiver(receiver).setPath(path)
        .setType(type).build();
    Request req = Request.newBuilder().setFuncValue(Functions.FUNC_SEND_XML_VALUE).setXml(xmlMsg)
        .build();
    logger.debug("sendXml: {}", bytesToHex(req.toByteArray()));
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
   * @param path     表情路径
   * @param receiver 消息接收者
   * @return 发送结果状态码
   */
  public int sendEmotion(String path, String receiver) {
    Wcf.PathMsg pathMsg = Wcf.PathMsg.newBuilder().setPath(path).setReceiver(receiver).build();
    Request req = Request.newBuilder().setFuncValue(Functions.FUNC_SEND_EMOTION_VALUE)
        .setFile(pathMsg).build();
    logger.debug("sendEmotion: {}", bytesToHex(req.toByteArray()));
    Response rsp = sendCmd(req);
    int ret = -1;
    if (rsp != null) {
      ret = rsp.getStatus();
    }

    return ret;
  }

  /**
   * 接收好友请求
   *
   * @param v3 xml.attrib["encryptusername"]
   * @param v4 xml.attrib["ticket"]
   * @return 结果状态码
   */
  public int acceptNewFriend(String v3, String v4) {
    int ret = -1;
    Verification verification = Verification.newBuilder().setV3(v3).setV4(v4).build();
    Request req = Request.newBuilder().setFuncValue(Functions.FUNC_ACCEPT_FRIEND_VALUE)
        .setV(verification).build();
    Response rsp = sendCmd(req);
    if (rsp != null) {
      ret = rsp.getStatus();
    }
    return ret;
  }

  /**
   * 添加群成员为微信好友
   *
   * @param roomID 群ID
   * @param wxIds  要加群的人列表，逗号分隔
   * @return 1 为成功，其他失败
   */
  public int addChatroomMembers(String roomID, String wxIds) {
    int ret = -1;
    MemberMgmt memberMgmt = MemberMgmt.newBuilder().setRoomid(roomID).setWxids(wxIds).build();
    Request req = Request.newBuilder().setFuncValue(Functions.FUNC_ADD_ROOM_MEMBERS_VALUE)
        .setM(memberMgmt).build();
    Response rsp = sendCmd(req);
    if (rsp != null) {
      ret = rsp.getStatus();
    }
    return ret;
  }

  /**
   * 解密图片
   *
   * @param srcPath 加密的图片路径
   * @param dstPath 解密的图片路径
   * @return 是否成功
   */
  public boolean decryptImage(String srcPath, String dstPath) {
    int ret = -1;
    DecPath build = DecPath.newBuilder().setSrc(srcPath).setDst(dstPath).build();
    Request req = Request.newBuilder().setFuncValue(Functions.FUNC_DECRYPT_IMAGE_VALUE)
        .setDec(build).build();
    Response rsp = sendCmd(req);
    if (rsp != null) {
      ret = rsp.getStatus();
    }
    return ret == 1;
  }

  /**
   * 获取个人信息
   *
   * @return 个人信息
   */
  public UserInfo getUserInfo() {
    Request req = Request.newBuilder().setFuncValue(Functions.FUNC_GET_USER_INFO_VALUE).build();
    Response rsp = sendCmd(req);
    if (rsp != null) {
      return rsp.getUi();
    }
    return null;
  }

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
   * @param wxMsgXml
   * @param wxMsgContent
   * @return
   */
  public boolean isAtMeMsg(String wxMsgXml, String wxMsgContent) {
    String format = String.format("<atuserlist><![CDATA[%s]]></atuserlist>", getSelfWxid());
    boolean isAtAll = wxMsgContent.startsWith("@所有人") || wxMsgContent.startsWith("@all");
    if (wxMsgXml.contains(format) && !isAtAll) {
      return true;
    }
    return false;
  }

  private void listenMsg(String url) {
    try {
      msgSocket = new Pair1Socket();
      msgSocket.dial(url);
      msgSocket.setReceiveTimeout(2000); // 2 秒超时
    } catch (Exception e) {
      logger.error("创建消息 RPC 失败", e);
      return;
    }
    ByteBuffer bb = ByteBuffer.allocate(BUFFER_SIZE);
    while (isReceivingMsg) {
      try {
        long size = msgSocket.receive(bb, true);
        WxMsg wxMsg = Response.parseFrom(Arrays.copyOfRange(bb.array(), 0, (int) size)).getWxmsg();
        msgQ.put(wxMsg);
      } catch (Exception e) {
        // 多半是超时，忽略吧
      }
    }
    try {
      msgSocket.close();
    } catch (Exception e) {
      logger.error("关闭连接失败", e);
    }
  }

  public void enableRecvMsg(int qSize) {
    if (isReceivingMsg) {
      return;
    }

    Request req = Request.newBuilder().setFuncValue(Functions.FUNC_ENABLE_RECV_TXT_VALUE).build();
    Response rsp = sendCmd(req);
    if (rsp == null) {
      logger.error("启动消息接收失败");
      isReceivingMsg = false;
      return;
    }

    isReceivingMsg = true;
    msgQ = new ArrayBlockingQueue<>(qSize);
    String msgUrl = "tcp://" + this.host + ":" + (this.port + 1);
    Thread thread = new Thread(() -> listenMsg(msgUrl));
    thread.start();
  }

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

  public void printContacts(List<RpcContact> contacts) {
    for (RpcContact c : contacts) {
      int value = c.getGender();
      String gender;
      if (value == 1) {
        gender = "男";
      } else if (value == 2) {
        gender = "女";
      } else {
        gender = "未知";
      }

      logger.info("{}, {}, {}, {}, {}, {}, {}", c.getWxid(), c.getName(), c.getCode(),
          c.getCountry(), c.getProvince(), c.getCity(), gender);
    }
  }

  public void printWxMsg(WxMsg msg) {
    logger.info("{}[{}]:{}:{}:{}\n{}", msg.getSender(), msg.getRoomid(), msg.getId(), msg.getType(),
        msg.getXml().replace("\n", "").replace("\t", ""), msg.getContent());
  }

  private String bytesToHex(byte[] bytes) {
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
}

