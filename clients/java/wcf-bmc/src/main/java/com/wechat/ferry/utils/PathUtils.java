/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package com.wechat.ferry.utils;

/**
 * @date 2025-05-02
 * @author zm
 */
import java.io.File;

public class PathUtils {

    /**
     * 分离文件路径，获取不带扩展名的部分 等同于 Python 的 os.path.splitext(thumb)[0]
     */
    public static String removeExtension(String path) {
        int dotIndex = path.lastIndexOf('.');
        if (dotIndex == -1) {
            return path;
        }
        return path.substring(0, dotIndex);
    }

    /**
     * 获取文件名（包含扩展名） 等同于 Python 的 os.path.basename(file_path)
     */
    public static String getFileName(String path) {
        File file = new File(path);
        return file.getName();
    }

    /**
     * 不存在则创建
     * @param dirPath 
     * @return  
     */
    public static boolean createDir(String dirPath) {
        File dir = new File(dirPath);
        if (!dir.exists()) {
            boolean created = dir.mkdirs(); // 创建目录，包括任何不存在的父目录
            if (created) {
                System.out.println("目录创建成功: " + dirPath);
            } else {
                System.out.println("目录创建失败: " + dirPath);
                return false;
            }
        } else {
            //Nothing to do
        }
        return true;
    }

    // 示例用法
    public static void main(String[] args) {
        String thumb = "/tmp/video_cover.jpg";

        String base = removeExtension(thumb);          // /tmp/video_cover
        String filePath = base + ".mp4";               // /tmp/video_cover.mp4
        String fileName = getFileName(filePath);       // video_cover.mp4

        System.out.println("base: " + base);
        System.out.println("filePath: " + filePath);
        System.out.println("fileName: " + fileName);
    }
}
