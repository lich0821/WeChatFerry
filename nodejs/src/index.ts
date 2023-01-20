// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import * as readline from 'readline';
import { stdin as input } from 'node:process';
import { WxMsg } from '../proto/wcf_pb';
import { log } from './utils/log';
import { handleCommand } from './commands';
import { Context } from './commands/base-command';
import { SDK } from './wcf/win32';
import { WCF } from './wcf';
import { portAvailable, sleep } from './utils/misic';

let ctx = new Context();

(async () => {
  if (await portAvailable(10086)) {
    log.debug(`Call SDK to inject.`);
    const sdk = new SDK();
    sdk.WxInitSDK();
  }

  // wait util WCF is ready
  while (!(await portAvailable(10086))) {
    await sleep(1000);
  }

  log.debug(`Connect to gRPC and wait for login.`);
  const wcf = new WCF();
  while (!(await wcf.IsLogin())) {
    await sleep(1000);
  }

  wcf.on('message', async (msg: WxMsg) => {
    const txt = msg.getContent();
    log.debug(txt, msg.getIsSelf());
    if (!msg.getIsSelf()) {
      const reply = await handleCommand(ctx, txt);
      if (msg.getIsGroup()) {
        wcf.SendTextMsg(msg.getRoomid(), reply);
      } else {
        wcf.SendTextMsg(msg.getSender(), reply);
      }
    }
  });

  wcf.EnableReceiveMsg();
})();

const std = readline.createInterface({ input });
std.on('line', async (line) => {
  const reply = await handleCommand(ctx, line);
  log.debug(`reply: ${reply}`);
  if (line === 'exit') {
    std.close();
  }
});
