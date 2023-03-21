package com.iamteer;

import com.iamteer.Wcf.DbNames;
import com.iamteer.Wcf.DbTable;
import com.iamteer.Wcf.Functions;
import com.iamteer.Wcf.Request;
import com.iamteer.Wcf.Response;
import com.iamteer.Wcf.RpcContact;

import io.sisu.nng.Socket;
import io.sisu.nng.pair.Pair1Socket;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class Client {
    private static final Logger logger = LoggerFactory.getLogger(Client.class);
    private final int BUFFER_SIZE = 16 * 1024 * 1024; // 16M
    private Socket socket = null;

    public Client(String hostPort) {
        connectRPC(hostPort);
    }

    private void connectRPC(String url) {
        try {
            socket = new Pair1Socket();
            socket.dial(url);
            logger.info("请点击登录微信");
            while (!isLogin()) { // 直到登录成功
                waitMs(1000);
            }
        } catch (Exception e) {
            logger.error("连接 RPC 失败: ", e);
            System.exit(-1);
        }
    }

    private Response sendCmd(Request req) {
        try {
            ByteBuffer bb = ByteBuffer.wrap(req.toByteArray());
            socket.send(bb);
            ByteBuffer ret = ByteBuffer.allocate(BUFFER_SIZE);
            long size = socket.receive(ret, true);
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
     * @Description 发送文本消息
     * @param msg:      消息内容（如果是 @ 消息则需要有跟 @ 的人数量相同的 @）
     * @param receiver: 消息接收人，私聊为 wxid（wxid_xxxxxxxxxxxxxx），群聊为
     *                  roomid（xxxxxxxxxx@chatroom）
     * @param aters:    群聊时要 @ 的人（私聊时为空字符串），多个用逗号分隔。@所有人 用
     *                  notify@all（必须是群主或者管理员才有权限）
     * @return int
     * @author Changhua
     * @example sendText("Hello @某人1 @某人2", "xxxxxxxx@chatroom",
     *          "wxid_xxxxxxxxxxxxx1,wxid_xxxxxxxxxxxxx2");
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

            logger.info("{}, {}, {}, {}, {}, {}, {}", c.getWxid(), c.getName(), c.getCode(), c.getCountry(),
                    c.getProvince(), c.getCity(), gender);
        }
    }

    public String bytesToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder();
        for (byte b : bytes) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }
}
