package com.iamteer;

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
}
