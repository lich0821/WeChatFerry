// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
import { log } from '../utils/log';
import { Interaction, Command, Context } from './base-command';

function RandomNum(max: number): number {
  return Math.floor(Math.random() * (max + 1));
}

const keywords = ['rock', 'paper', 'scissors'];

export default {
  name: 'rock-scissors-paper',
  follow: (interaction: Interaction): number => {
    const i = keywords.find((k) => {
      return interaction.input.includes(k);
    });
    log.debug(`rock-scissors-paper follow-up: ${i}`);
    interaction.options.pick = i ?? undefined;
    return i ? 1 : 0;
  },
  run: (ctx: Context, interaction: Interaction): string => {
    const pick: string = interaction.options.pick as string;

    const playerPick = keywords.findIndex((x) => x == pick);
    const botPick = RandomNum(2);

    let result = 'tied';

    if (playerPick == botPick) result = 'Tied';
    else if (playerPick == 0 && botPick == 2) result = 'Win';
    else if ((playerPick - botPick) % 3 == 1) result = 'Win';
    else result = 'Lose';

    const content = `You picked **${pick}** and I picked **${keywords[botPick]}**, You **${result}**`;
    return content;
  }
} as Command;
