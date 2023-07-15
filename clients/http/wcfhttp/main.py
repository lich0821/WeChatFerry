#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
import argparse

import uvicorn
from wcferry import Wcf
from wcfhttp import Http, __version__


def main():
    parse = argparse.ArgumentParser()
    parse.add_argument("-v", "--version", action="version", version=f"{__version__}")
    parse.add_argument("--wcf_host", type=str, default=None, help="WeChatFerry 监听地址，默认本地启动监听 0.0.0.0")
    parse.add_argument("--wcf_port", type=int, default=10086, help="WeChatFerry 监听端口 (同时占用 port + 1 端口)，默认 10086")
    parse.add_argument("--wcf_debug", type=bool, default=False, help="是否打开 WeChatFerry 调试开关")
    parse.add_argument("--host", type=str, default="0.0.0.0", help="wcfhttp 监听地址，默认监听 0.0.0.0")
    parse.add_argument("--port", type=int, default=9999, help="wcfhttp 监听端口，默认 9999")
    parse.add_argument("--cb", type=str, default="", help="接收消息回调地址")

    logging.basicConfig(level="INFO", format="%(asctime)s %(message)s")
    args = parse.parse_args()
    cb = args.cb
    if not cb:
        logging.warning("没有设置接收消息回调，消息直接通过日志打印；请通过 --cb 设置消息回调")
        logging.warning(f"回调接口规范参考接收消息回调样例：http://{args.host}:{args.port}/docs")

    wcf = Wcf(args.wcf_host, args.wcf_port, args.wcf_debug)
    home = "https://github.com/lich0821/WeChatFerry"
    http = Http(wcf=wcf,
                cb=cb,
                title="WeChatFerry HTTP 客户端",
                description=f"Github: <a href='{home}'>WeChatFerry</a>",)

    uvicorn.run(app=http, host=args.host, port=args.port)


if __name__ == "__main__":
    main()
