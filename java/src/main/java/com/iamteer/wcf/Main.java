package com.iamteer.wcf;

import java.util.List;
import java.util.Map;
import java.util.Iterator;

public class Main {
    public static void main(String[] args) throws Exception {
        String hostPort = "localhost:10086";
        Client client = new Client();
        client.InitClient(hostPort);
        System.out.println("Connecting to " + hostPort);

        int status = client.IsLogin();
        System.out.println(status);

        String wxid = client.GetSelfWxid();
        System.out.println(wxid);

        client.EnableRecvMsg(); // Receive Message

        Map<Integer, String> msgTypes = client.GetMsgTypes();
        Iterator<Map.Entry<Integer, String>> iterTypes = msgTypes.entrySet().iterator();
        while (iterTypes.hasNext()) {
            Map.Entry<Integer, String> entry = iterTypes.next();
            System.out.println(entry.getKey() + ": " + entry.getValue());
        }

        List<WcfOuterClass.Contact> contacts = client.GetContacts();
        Iterator<WcfOuterClass.Contact> iterContacts = contacts.iterator();
        while (iterContacts.hasNext()) {
            WcfOuterClass.Contact contact = iterContacts.next();
            System.out.println(contact);
        }

        List<String> dbs = client.GetDbs();
        Iterator<String> iterDbs = dbs.iterator();
        while (iterDbs.hasNext()) {
            String db = iterDbs.next();
            System.out.println(db);
        }

        List<WcfOuterClass.DbTable> tables = client.GetTables("MicroMsg.db");
        Iterator<WcfOuterClass.DbTable> iterTables = tables.iterator();
        while (iterTables.hasNext()) {
            WcfOuterClass.DbTable table = iterTables.next();
            System.out.println(table);
        }

        List<WcfOuterClass.DbRow> rows = client.QuerySql("MicroMsg.db", "SELECT * FROM Contact LIMIT 1;");
        Iterator<WcfOuterClass.DbRow> iterRows = rows.iterator();
        while (iterRows.hasNext()) {
            WcfOuterClass.DbRow row = iterRows.next();
            System.out.println(row);
        }

        // status = client.AcceptNewFriend("v3", "v4"); // 需要真实的数据
        // System.out.println(status);

        Thread.sleep(1000);
        client.CleanupClient();
    }

}
