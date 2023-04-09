package com.iamteer;

import com.iamteer.Wcf.*;
import io.sisu.nng.Socket;
import io.sisu.nng.pair.Pair1Socket;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.net.URL;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

public class Client {
    private static final Logger logger = LoggerFactory.getLogger(Client.class);
    private final int BUFFER_SIZE = 16 * 1024 * 1024; // 16M
    private Socket cmdSocket = null;
    private Socket msgSocket = null;
    private String host = "127.0.0.1";
    private int port = 10086;
    private boolean isReceivingMsg = false;
    private boolean isLocalHostPort = false;
    private BlockingQueue<WxMsg> msgQ;
    private String wcfPath;

    public Client(String host, int port) {
        this.host = host;
        this.port = port;
        String cmdUrl = "tcp://" + host + ":" + port;
        connectRPC(cmdUrl);
    }

    public Client(int port, boolean debug) {
        initClient(this.host, port, debug);
    }

    public Client(boolean debug) {
        initClient(this.host, this.port, debug);
    }

    private void initClient(String host, int port, boolean debug) {
        try {
            URL url = this.getClass().getResource("/win32-x86-64/wcf.exe");
            wcfPath = url.getFile();
            String[] cmd = new String[4];
            cmd[0] = wcfPath;
            cmd[1] = "start";
            cmd[2] = Integer.toString(port);
            if (debug) {
                cmd[3] = "debug";
            } else {
                cmd[3] = "";
            }
            int status = Runtime.getRuntime().exec(cmd).waitFor();
            if (status != 0) {
                logger.error("启动 RPC 失败: {}", status);
                System.exit(-1);
            }
            isLocalHostPort = true;
            String cmdUrl = "tcp://" + host + ":" + port;
            connectRPC(cmdUrl);
        } catch (Exception e) {
            logger.error("初始化失败: {}", e);
            System.exit(-1);
        }
    }

    private void connectRPC(String url) {
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
        Runtime.getRuntime().addShutdownHook(new Thread() {
            public void run() {
                logger.info("关闭...");
                diableRecvMsg();
                if (isLocalHostPort) {
                    try {
                        String[] cmd = new String[2];
                        cmd[0] = wcfPath;
                        cmd[1] = "stop";
                        Process process = Runtime.getRuntime().exec(cmd);
                        int status = process.waitFor();
                        if (status != 0) {
                            System.err.println("停止机器人失败: " + status);
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            }
        });
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

    public boolean isLogin() {
        Request req = new Request.Builder().setFuncValue(Functions.FUNC_IS_LOGIN_VALUE).build();
        Response rsp = sendCmd(req);
        if (rsp != null) {
            return rsp.getStatus() == 1;
        }
        return false;
    }

    public String getSelfWxid() {
        Request req = new Request.Builder().setFuncValue(Functions.FUNC_GET_SELF_WXID_VALUE).build();
        Response rsp = sendCmd(req);
        if (rsp != null) {
            return rsp.getStr();
        }

        return "";
    }

    public Map<Integer, String> getMsgTypes() {
        Request req = new Request.Builder().setFuncValue(Functions.FUNC_GET_MSG_TYPES_VALUE).build();
        Response rsp = sendCmd(req);
        if (rsp != null) {
            return rsp.getTypes().getTypesMap();
        }

        return Wcf.MsgTypes.newBuilder().build().getTypesMap();
    }

    public List<RpcContact> getContacts() {
        Request req = new Request.Builder().setFuncValue(Functions.FUNC_GET_CONTACTS_VALUE).build();
        Response rsp = sendCmd(req);
        if (rsp != null) {
            return rsp.getContacts().getContactsList();
        }

        return Wcf.RpcContacts.newBuilder().build().getContactsList();
    }

    public List<String> getDbNames() {
        Request req = new Request.Builder().setFuncValue(Functions.FUNC_GET_DB_NAMES_VALUE).build();
        Response rsp = sendCmd(req);
        if (rsp != null) {
            return rsp.getDbs().getNamesList();
        }

        return Wcf.DbNames.newBuilder().build().getNamesList();
    }

    public Map<String, String> getDbTables(String db) {
        Request req = new Request.Builder().setFuncValue(Functions.FUNC_GET_DB_TABLES_VALUE).setStr(db).build();
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
        Wcf.TextMsg textMsg = Wcf.TextMsg.newBuilder().setMsg(msg).setReceiver(receiver).setAters(aters).build();
        Request req = new Request.Builder().setFuncValue(Functions.FUNC_SEND_TXT_VALUE).setTxt(textMsg).build();
        logger.debug("sendText: {}", bytesToHex(req.toByteArray()));
        Response rsp = sendCmd(req);
        int ret = -1;
        if (rsp != null) {
            ret = rsp.getStatus();
        }

        return ret;
    }

    public int sendImage(String path, String receiver) {
        Wcf.PathMsg pathMsg = Wcf.PathMsg.newBuilder().setPath(path).setReceiver(receiver).build();
        Request req = new Request.Builder().setFuncValue(Functions.FUNC_SEND_IMG_VALUE).setFile(pathMsg).build();
        logger.debug("sendImage: {}", bytesToHex(req.toByteArray()));
        Response rsp = sendCmd(req);
        int ret = -1;
        if (rsp != null) {
            ret = rsp.getStatus();
        }

        return ret;
    }

    public int sendFile(String path, String receiver) {
        Wcf.PathMsg pathMsg = Wcf.PathMsg.newBuilder().setPath(path).setReceiver(receiver).build();
        Request req = new Request.Builder().setFuncValue(Functions.FUNC_SEND_FILE_VALUE).setFile(pathMsg).build();
        logger.debug("sendFile: {}", bytesToHex(req.toByteArray()));
        Response rsp = sendCmd(req);
        int ret = -1;
        if (rsp != null) {
            ret = rsp.getStatus();
        }

        return ret;
    }

    public int sendXml(String receiver, String xml, String path, int type) {
        Wcf.XmlMsg xmlMsg = Wcf.XmlMsg.newBuilder().setContent(xml).setReceiver(receiver).setPath(path).setType(type).build();
        Request req = new Request.Builder().setFuncValue(Functions.FUNC_SEND_XML_VALUE).setXml(xmlMsg).build();
        logger.debug("sendXml: {}", bytesToHex(req.toByteArray()));
        Response rsp = sendCmd(req);
        int ret = -1;
        if (rsp != null) {
            ret = rsp.getStatus();
        }

        return ret;
    }

    public int sendEmotion(String path, String receiver) {
        Wcf.PathMsg pathMsg = Wcf.PathMsg.newBuilder().setPath(path).setReceiver(receiver).build();
        Request req = new Request.Builder().setFuncValue(Functions.FUNC_SEND_EMOTION_VALUE).setFile(pathMsg).build();
        logger.debug("sendEmotion: {}", bytesToHex(req.toByteArray()));
        Response rsp = sendCmd(req);
        int ret = -1;
        if (rsp != null) {
            ret = rsp.getStatus();
        }

        return ret;
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

    private void listenMsg(String url) {
        try {
            msgSocket = new Pair1Socket();
            msgSocket.dial(url);
            msgSocket.setReceiveTimeout(2000); // 2 秒超时
        } catch (Exception e) {
            logger.error("创建消息 RPC 失败: {}", e);
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
            logger.error("关闭连接失败: {}", e);
        }
    }

    public void enableRecvMsg(int qSize) {
        if (isReceivingMsg) {
            return;
        }

        Request req = new Request.Builder().setFuncValue(Functions.FUNC_ENABLE_RECV_TXT_VALUE).build();
        Response rsp = sendCmd(req);
        if (rsp == null) {
            logger.error("启动消息接收失败");
            isReceivingMsg = false;
            return;
        }

        isReceivingMsg = true;
        msgQ = new ArrayBlockingQueue<WxMsg>(qSize);
        String msgUrl = "tcp://" + this.host + ":" + (this.port + 1);
        Thread thread = new Thread(new Runnable() {
            public void run() {
                listenMsg(msgUrl);
            }
        });
        thread.start();
    }

    public int diableRecvMsg() {
        if (!isReceivingMsg) {
            return 1;
        }
        int ret = -1;
        Request req = new Request.Builder().setFuncValue(Functions.FUNC_DISABLE_RECV_TXT_VALUE).build();
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

            logger.info("{}, {}, {}, {}, {}, {}, {}", c.getWxid(), c.getName(), c.getCode(), c.getCountry(), c.getProvince(), c.getCity(), gender);
        }
    }

    public void printWxMsg(WxMsg msg) {
        logger.info("{}[{}]:{}:{}:{}\n{}", msg.getSender(), msg.getRoomid(), msg.getId(), msg.getType(), msg.getXml().replace("\n", "").replace("\t", ""), msg.getContent());
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
}
