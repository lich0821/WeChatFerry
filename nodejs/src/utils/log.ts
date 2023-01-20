// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import { Logger } from 'tslog';

export const log: Logger = new Logger({
  name: 'main',
  displayLoggerName: false,
  overwriteConsole: true,
  dateTimePattern: 'monthday hour:minute:second'
});
