package com.wechat.ferry.utils;

import java.io.File;
import java.io.FileInputStream;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.util.List;

import org.dom4j.Attribute;
import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.DocumentHelper;
import org.dom4j.Element;

import com.alibaba.fastjson2.JSONArray;
import com.alibaba.fastjson2.JSONObject;

import lombok.extern.slf4j.Slf4j;

@Slf4j
public class XmlJsonConvertUtil {

    public static String readFile(String path) {
        String str = "";
        try {
            File file = new File(path);
            FileInputStream fis = new FileInputStream(file);
            FileChannel fc = fis.getChannel();
            ByteBuffer bb = ByteBuffer.allocate(new Long(file.length()).intValue());
            // fc向buffer中读入数据
            fc.read(bb);
            bb.flip();
            str = new String(bb.array(), "UTF8");
            fc.close();
            fis.close();
        } catch (Exception e) {
            log.error("异常：{} ", e.getMessage());
        }
        return str;
    }

    /**
     * xml转json
     * 
     * @param xmlStr
     * @return
     */
    public static JSONObject xml2Json(String xmlStr) {
        JSONObject json = new JSONObject();
        try {
            xmlStr = xmlStr.replace("<?xml version=\\\"1.0\\\"?>\\n", "");
            Document doc = DocumentHelper.parseText(xmlStr);
            dom4j2Json(doc.getRootElement(), json);
        } catch (DocumentException e) {
            log.error("异常：{} ", e.getMessage());
        }
        return json;
    }

    /**
     * xml转json
     * 
     * @param element
     * @param json
     */
    public static void dom4j2Json(Element element, JSONObject json) {
        // 如果是属性
        for (Object o : element.attributes()) {
            Attribute attr = (Attribute)o;
            if (!isEmpty(attr.getValue())) {
                json.put("@" + attr.getName(), attr.getValue());
            }
        }
        List<Element> chdEl = element.elements();
        if (chdEl.isEmpty() && !isEmpty(element.getText())) {
            // 如果没有子元素,只有一个值
            json.put(element.getName(), element.getText());
        }

        for (Element e : chdEl) {
            // 有子元素
            if (!e.elements().isEmpty()) {
                // 子元素也有子元素
                JSONObject chdjson = new JSONObject();
                dom4j2Json(e, chdjson);
                Object o = json.get(e.getName());
                if (o != null) {
                    JSONArray jsona = null;
                    if (o instanceof JSONObject) {
                        // 如果此元素已存在,则转为jsonArray
                        JSONObject jsono = (JSONObject)o;
                        json.remove(e.getName());
                        jsona = new JSONArray();
                        jsona.add(jsono);
                        jsona.add(chdjson);
                    }
                    if (o instanceof JSONArray) {
                        jsona = (JSONArray)o;
                        jsona.add(chdjson);
                    }
                    json.put(e.getName(), jsona);
                } else {
                    if (!chdjson.isEmpty()) {
                        json.put(e.getName(), chdjson);
                    }
                }

            } else {
                // 子元素没有子元素
                for (Object o : element.attributes()) {
                    Attribute attr = (Attribute)o;
                    if (!isEmpty(attr.getValue())) {
                        json.put("@" + attr.getName(), attr.getValue());
                    }
                }
                if (!e.getText().isEmpty()) {
                    json.put(e.getName(), e.getText());
                }
            }
        }
    }

    public static boolean isEmpty(String str) {
        if (str == null || str.trim().isEmpty() || "null".equals(str)) {
            return true;
        }
        return false;
    }

}
