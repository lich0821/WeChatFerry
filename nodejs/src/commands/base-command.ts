// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

export class Context {}

export class Interaction {
  constructor(input: string) {
    this.input = input;
    this.options = {};
  }
  input: string;
  output?: string;
  options: any;
}

export interface Command {
  name: string;
  follow: (interaction: Interaction) => number;
  run: (ctx: Context, interaction: Interaction) => string | Promise<string>;
}
