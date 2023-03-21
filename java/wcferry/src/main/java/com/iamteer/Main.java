package com.iamteer;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class Main {
    private static Logger logger = LoggerFactory.getLogger(Main.class);

    public static void main(String[] args) {
        final String url = "tcp://192.168.1.104:10086";
        Client client = new Client(url);

        // 是否已登录
        logger.info("isLogin: {}", client.isLogin());

        // 登录账号 wxid
        logger.info("wxid: {}", client.getSelfWxid());

        // 消息类型
        logger.info("message types: {}", client.getMsgTypes());

        // 所有联系人（包括群聊、公众号、好友……）
        client.printContacts(client.getContacts());

        // 获取数据库
        logger.info("dbs: {}", client.getDbNames());

        // 获取数据库下的表
        String db = "MicroMsg.db";
        logger.info("tables in {}: {}", db, client.getDbTables(db));

        // 发送文本消息，aters 是要 @ 的 wxid，多个用逗号分隔；消息里@的数量要与aters里的数量对应
        client.sendText("Hello", "filehelper", "");
        // client.sendText("Hello @某人1 @某人2", "xxxxxxxx@chatroom", "wxid_xxxxxxxxxxxxx1,wxid_xxxxxxxxxxxxx2");

        // 发送图片消息，图片必须要存在
        client.sendImage("C:\\Projs\\WeChatFerry\\TEQuant.jpeg", "filehelper");
    }
}
