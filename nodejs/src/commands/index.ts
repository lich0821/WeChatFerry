// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import { log } from '../utils/log';
import { Interaction, Command, Context } from './base-command';
import rsp from './rock-scissors-paper';
import stock from './stock';

let commands: Command[] = [rsp, stock];

export const handleCommand = async (ctx: Context, input: string): Promise<string> => {
  const interaction = new Interaction(input);
  const choice = commands
    .map((c) => {
      return { score: c.follow(interaction), command: c };
    })
    ?.sort((a, b) => {
      return a.score - b.score;
    })
    ?.filter((x) => x.score > 0)
    ?.pop();

  if (!choice) {
    log.debug('no command found');
    return '';
  }
  const reply = await choice.command.run(ctx, interaction);
  return reply;
};
