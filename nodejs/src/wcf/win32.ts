// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
import { Library, Method, never } from 'ffi-decorators';

@Library({ libPath: './sdk.dll' })
export class SDK {
  @Method({ types: ['int', []] })
  WxInitSDK(): number {
    return never();
  }

  @Method({ types: ['int', []] })
  WxDestroySDK(): number {
    return never();
  }
}

// Todo: inject spy.dll directly in ts code.
