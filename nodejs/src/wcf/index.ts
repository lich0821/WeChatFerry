// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import EventEmitter from 'events';
import TypedEmitter from 'typed-emitter';
import * as grpc from 'grpc';
import { WcfClient, IWcfClient } from '../../proto/wcf_grpc_pb';
import { Empty, TextMsg, WxMsg } from '../../proto/wcf_pb';

type MessageEvents = {
  error: (error: Error) => void;
  message: (msg: WxMsg) => void;
};

export class WCF extends (EventEmitter as new () => TypedEmitter<MessageEvents>) {
  client: IWcfClient;

  constructor() {
    super();
    this.client = new WcfClient(`localhost:10086`, grpc.credentials.createInsecure());
  }

  async IsLogin(): Promise<boolean> {
    return new Promise<boolean>((resolve, reject) => {
      const request = new Empty();
      this.client.rpcIsLogin(request, (err, ret) => {
        if (err) reject(err);
        else resolve(ret.getStatus() !== 0);
      });
    });
  }

  async EnableReceiveMsg() {
    let stream = this.client.rpcEnableRecvMsg(new Empty());
    stream.on('data', (d) => {
      this.emit('message', d);
    });
    stream.on('error', (err) => {
      this.emit('error', err);
    });
  }

  async SendTextMsg(to: string, msg: string): Promise<boolean> {
    return new Promise<boolean>((resolve, reject) => {
      const request = new TextMsg();
      request.setReceiver(to);
      request.setMsg(msg);

      this.client.rpcSendTextMsg(request, (err, ret) => {
        if (err) reject(err);
        else resolve(ret.getStatus() !== 0);
      });
    });
  }
}
