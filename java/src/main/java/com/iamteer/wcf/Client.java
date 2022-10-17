package com.iamteer.wcf;

import java.util.List;
import java.util.Map;
import com.sun.jna.Library;
import com.sun.jna.Native;

import io.grpc.ManagedChannel;
import io.grpc.stub.StreamObserver;
import io.grpc.ManagedChannelBuilder;

public class Client {
    private interface JnaLibrary extends Library {
        JnaLibrary INSTANCE = Native.load("sdk", JnaLibrary.class);

        void WxInitSDK();

        void WxDestroySDK();
    }

    public void InitClient(String hostPort) {
        JnaLibrary.INSTANCE.WxInitSDK();
        this.connect(hostPort);
    }

    public void CleanupClient() {
        this.DisableRecvMsg();
        JnaLibrary.INSTANCE.WxDestroySDK();
    }

    private void connect(String hostPort) {
        ManagedChannel managedChannel = ManagedChannelBuilder.forTarget(hostPort)
                .usePlaintext()
                .build();
        this.wcfBlokingStub = WcfGrpc.newBlockingStub(managedChannel);
        this.wcfStub = WcfGrpc.newStub(managedChannel);
    }

    public int IsLogin() {
        WcfOuterClass.Empty empty = WcfOuterClass.Empty.newBuilder().build();
        WcfOuterClass.Response response = this.wcfBlokingStub.rpcIsLogin(empty);
        return response.getStatus();
    }

    public String GetSelfWxid() {
        WcfOuterClass.Empty empty = WcfOuterClass.Empty.newBuilder().build();
        WcfOuterClass.String rsp = this.wcfBlokingStub.rpcGetSelfWxid(empty);
        return rsp.getStr();
    }

    public void EnableRecvMsg() {
        if (isReceivingMsg) {
            return;
        }

        isReceivingMsg = true;
        WcfOuterClass.Empty empty = WcfOuterClass.Empty.newBuilder().build();
        this.wcfStub.rpcEnableRecvMsg(empty, new StreamObserver<WcfOuterClass.WxMsg>() {
            @Override
            public void onNext(WcfOuterClass.WxMsg value) {
                System.out.printf("New Message:\n%s", value);
            }

            @Override
            public void onError(Throwable t) {
                System.err.println("EnableRecvMsg Error");
            }

            @Override
            public void onCompleted() {
                System.out.println("EnableRecvMsg Complete");
            }
        });
    }

    public int DisableRecvMsg() {
        if (!isReceivingMsg) {
            return 0;
        }

        WcfOuterClass.Empty empty = WcfOuterClass.Empty.newBuilder().build();
        WcfOuterClass.Response response = this.wcfBlokingStub.rpcDisableRecvMsg(empty);
        if (response.getStatus() == 0) {
            isReceivingMsg = false;
            return 0;
        }

        return -1;
    }

    public int SendText(String msg, String receiver, String aters) {
        WcfOuterClass.TextMsg textMsg = WcfOuterClass.TextMsg.newBuilder().setMsg(msg).setReceiver(receiver)
                .setAters(aters).build();
        WcfOuterClass.Response response = this.wcfBlokingStub.rpcSendTextMsg(textMsg);
        return response.getStatus();
    }

    public int SendImage(String path, String receiver) {
        WcfOuterClass.ImageMsg imageMsg = WcfOuterClass.ImageMsg.newBuilder().setPath(path).setReceiver(receiver)
                .build();
        WcfOuterClass.Response response = this.wcfBlokingStub.rpcSendImageMsg(imageMsg);
        return response.getStatus();
    }

    public Map<Integer, String> GetMsgTypes() {
        WcfOuterClass.Empty empty = WcfOuterClass.Empty.newBuilder().build();
        WcfOuterClass.MsgTypes msgTypes = this.wcfBlokingStub.rpcGetMsgTypes(empty);
        return msgTypes.getTypesMap();
    }

    public List<WcfOuterClass.Contact> GetContacts() {
        WcfOuterClass.Empty empty = WcfOuterClass.Empty.newBuilder().build();
        WcfOuterClass.Contacts contacts = this.wcfBlokingStub.rpcGetContacts(empty);
        return contacts.getContactsList();
    }

    public List<String> GetDbs() {
        WcfOuterClass.Empty empty = WcfOuterClass.Empty.newBuilder().build();
        WcfOuterClass.DbNames dbs = this.wcfBlokingStub.rpcGetDbNames(empty);
        return dbs.getNamesList();
    }

    public List<WcfOuterClass.DbTable> GetTables(String db) {
        WcfOuterClass.String str = WcfOuterClass.String.newBuilder().setStr(db).build();
        WcfOuterClass.DbTables tables = this.wcfBlokingStub.rpcGetDbTables(str);
        return tables.getTablesList();
    }

    public List<WcfOuterClass.DbRow> QuerySql(String db, String sql) {
        WcfOuterClass.DbQuery query = WcfOuterClass.DbQuery.newBuilder().setDb(db).setSql(sql).build();
        WcfOuterClass.DbRows rows = this.wcfBlokingStub.rpcExecDbQuery(query);
        return rows.getRowsList();
    }

    public int AcceptNewFriend(String v3, String v4) {
        WcfOuterClass.Verification v = WcfOuterClass.Verification.newBuilder().setV3(v3).setV4(v4).build();
        WcfOuterClass.Response response = this.wcfBlokingStub.rpcAcceptNewFriend(v);
        return response.getStatus();
    }

    private boolean isReceivingMsg = false;
    private WcfGrpc.WcfBlockingStub wcfBlokingStub;
    private WcfGrpc.WcfStub wcfStub;
}
