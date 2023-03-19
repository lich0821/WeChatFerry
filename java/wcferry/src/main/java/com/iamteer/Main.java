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
    }
}
