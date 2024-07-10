package com.iamteer;

import com.sun.jna.Library;
import com.sun.jna.Native;

/**
 * SDK.dll的接口类
 * @Description
 * @Author xinggq
 * @Date 2024/7/10
 */
public interface SDK extends Library {
  // 读取项目根目录下dll文件夹，而且只能写绝对路径，放在resource下会有问题，暂时先不解决
  // 打包后，dll文件应该放在jar外，这样dll更新不需要生成jar包，重启下就ok了
  SDK INSTANCE = Native.load(System.getProperty("user.dir")+"\\dll\\sdk.dll", SDK.class);

  /**
   * 初始化SDK
   * @param debug
   * @param port
   * @return
   */
  int WxInitSDK(boolean debug, int port);

  /**
   * 退出SDK
   * @return
   */
  int WxDestroySDK();
}