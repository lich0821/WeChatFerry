#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import time
import wcferry as sdk


def main():
    help(sdk)  # 查看SDK支持的方法和属性

    # 初始化SDK，如果成功，返回0；否则失败
    status = sdk.WxInitSDK()
    if status != 0:
        print("初始化失败")
        exit(-1)

    print("初始化成功")
    WxMsgTypes = sdk.WxGetMsgTypes()    # 获取消息类型
    print(WxMsgTypes)                   # 查看消息类型

    time.sleep(2)
    print("打印通讯录......")
    contacts = sdk.WxGetContacts()
    for k, v in contacts.items():
        print(k, v.wxCode, v.wxName, v.wxCountry, v.wxProvince, v.wxCity, v.wxGender)

    time.sleep(2)
    print("发送文本消息......")
    sdk.WxSendTextMsg("filehelper", "message from WeChatFerry...")  # 往文件传输助手发消息
    # sdk.WxSendTextMsg("xxxx@chatroom", "message from WeChatFerry...")  # 往群里发消息（需要改成正确的 ID，下同）
    # sdk.WxSendTextMsg("xxxx@chatroom", "message from WeChatFerry... @ ", "wxid_xxxxxxxxxxxx") # 往群里发消息，@某人
    # sdk.WxSendTextMsg("xxxx@chatroom", "message from WeChatFerry... @ ", "notify@all")  # 往群里发消息，@所有人

    time.sleep(2)
    print("发送图片消息......")
    sdk.WxSendImageMsg("filehelper", "test.jpg")

    dbs = sdk.WxGetDbNames()
    for db in dbs:
        print(db)

    tables = sdk.WxGetDbTables(dbs[0])
    for t in tables:
        print(f"{t.table}\n{t.sql}\n\n")

    # 接收消息。先定义消息处理回调
    def OnTextMsg(msg: sdk.WxMessage):
        def getName(id):
            contact = contacts.get(id)
            if contact is None:
                return id
            return contact.wxName

        s = "收到"
        if msg.self == 1:  # 忽略自己发的消息
            s += f"来自自己的消息"
            print(f"\n{s}")
            return 0

        msgType = WxMsgTypes.get(msg.type, '未知类型')
        nickName = getName(msg.wxId)
        if msg.source == 1:
            groupName = getName(msg.roomId)
            s += f"来自群[{groupName}]的[{nickName}]的{msgType}消息："
        else:
            s += f"来自[{nickName}]的{msgType}消息："

        s += f"\r\n{msg.content}"
        if msg.type != 0x01:
            s += f"\r\n{msg.xml}"

        print(f"\n{s}")

        return 0

    print("Message: 接收通知中......")
    sdk.WxEnableRecvMsg(OnTextMsg)  # 设置回调，接收消息

    while True:
        time.sleep(1)


if __name__ == '__main__':
    main()
