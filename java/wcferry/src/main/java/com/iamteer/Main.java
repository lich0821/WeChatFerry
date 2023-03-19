package com.iamteer;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class Main {
    private static Logger logger = LoggerFactory.getLogger(Main.class);

    public static void main(String[] args) {
        final String url = "tcp://192.168.1.104:10086";
        Client client = new Client(url);

        logger.info("isLogin: {}", client.isLogin());
        logger.info("wxid: {}", client.getSelfWxid());
    }
}
