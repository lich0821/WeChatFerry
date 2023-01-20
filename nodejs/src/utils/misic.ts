// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

export function portAvailable(port: number): Promise<boolean> {
  const net = require('net');
  return new Promise((resolve, _reject) => {
    let server = net.createServer().listen(port);
    server.on('listening', function () {
      server.close();
      resolve(true);
    });
    server.on('error', function (err: any) {
      if (err.code == 'EADDRINUSE') {
        resolve(false);
      }
    });
  });
}

export const sleep = (ms: number) => new Promise((r) => setTimeout(r, ms));
