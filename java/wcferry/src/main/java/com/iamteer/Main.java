package com.iamteer;

public class Main {
    public static void main(String[] args) {
        final String url = "tcp://192.168.1.104:10086";
        Client client = new Client(url);

        System.out.println("IsLogin: " + client.isLogin());
    }
}
