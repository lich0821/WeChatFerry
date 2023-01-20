// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
import { stocks } from 'stock-api';
import { Interaction, Command, Context } from './base-command';

function getPrice(code: string): Promise<string> {
  return stocks.tencent.getStocks([code]).then((res) => {
    return JSON.stringify(res[0]);
  });
}

export default {
  name: 'stock',
  follow: (interaction: Interaction): number => {
    let match = /(S[HZ]\d{6})/.exec(interaction.input);
    if (match) {
      let stock = match && match[0];
      interaction.options.stock = stock;
      return 1;
    }
    return 0;
  },
  run: async (ctx: Context, interaction: Interaction): Promise<string> => {
    const pick: string = interaction.options.stock as string;
    const content = await getPrice(pick);
    return content;
  }
} as Command;
