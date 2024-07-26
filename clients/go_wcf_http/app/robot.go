package app

/*
#cgo LDFLAGS: -L../ -lsdk
#include <stdlib.h>
#include <stdbool.h>

extern int WxInitSDK(bool, int);
extern int WxDestroySDK();
*/
import "C"
import (
	"bytes"
	"fmt"
	"github.com/go-resty/resty/v2"
	"github.com/google/uuid"
	"go_wechatFerry/wcf"
	"io"
	"os"
	"path/filepath"
	"strings"
	"time"
)

var WxClient *wcf.Client

// Message 组装成一个结构体展示消息
type Message struct {
	IsSelf    bool   `json:"is_self,omitempty"`
	IsGroup   bool   `json:"is_group,omitempty"`
	MessageId uint64 `json:"message_id,omitempty"`
	Type      uint32 `json:"type,omitempty"`
	Ts        uint32 `json:"ts,omitempty"`
	RoomId    string `json:"room_id,omitempty"`
	Content   string `json:"content,omitempty"`
	WxId      string `json:"wx_id,omitempty"`
	Sign      string `json:"sign,omitempty"`
	Thumb     string `json:"thumb,omitempty"`
	Extra     string `json:"extra,omitempty"`
	Xml       string `json:"xml,omitempty"`
}

// WechatFerryInit 调用sdk.dll中的WxInitSdk 进行启动微信并注入
func WechatFerryInit() {
	// 调试模式  端口
	initSuccess := C.WxInitSDK(C.bool(false), C.int(10086))
	if initSuccess == 0 {
		fmt.Println("SDK 初始化成功")
	} else {
		fmt.Println("SDK 初始化失败")
	}
	time.Sleep(time.Millisecond * 5000)
	// 连接服务器
	client, errs := wcf.NewWCF("")
	if errs != nil {
		return
	}
	// 一定要在这里判断是否登录成功 否则会导致用户列表获取失败
	for true {
		if client.IsLogin() == true {
			fmt.Println("登录成功...等待初始化中...")
			time.Sleep(2000 * time.Millisecond)
			break
		}
		time.Sleep(1000 * time.Millisecond)
	}
	WxClient = client
	ContactsInit()
	fmt.Println("初始化完成")
}

// MessageProcess  在这里可以继续写代码了
func MessageProcess(msg Message) {
	// 方法都在WxClient中
	//WxClient.SendTxt("测试","","")
	fmt.Println(msg)
}

// ContactsInit 通讯录初始化
func ContactsInit() {
	var contactsMap []map[string]string
	contacts := WxClient.GetContacts()
	for _, v := range contacts {
		gender := ""
		if v.Gender == 1 {
			gender = "男"
		}
		if v.Gender == 2 {
			gender = "女"
		}
		contactsMaps := map[string]string{
			"wxId":     v.Wxid,
			"code":     v.Code,
			"remark":   v.Remark,
			"name":     v.Name,
			"country":  v.Country,
			"province": v.Province,
			"city":     v.City,
			"gender":   gender,
		}
		contactsMap = append(contactsMap, contactsMaps)
	}
	WxClient.ContactsMap = contactsMap
}

// DownloadFile 下载文件
func DownloadFile(url string, fileType string, suffix string) (string, error) {
	fmt.Println(url)
	// 发送HTTP请求获取文件
	resp, err := resty.New().R().Get(url)
	if err != nil {
		return "", err
	}

	// 获取当前日期
	currentTime := time.Now()
	datePath := filepath.Join("./resource/static/"+fileType, currentTime.Format("2006-01-02"))
	// 创建目录
	if err := os.MkdirAll(datePath, os.ModePerm); err != nil {
		return "", err
	}

	// 生成唯一的文件名
	fileName := uuid.New().String() + "." + suffix
	filePath := filepath.Join(datePath, fileName)

	// 创建文件
	file, err := os.Create(filePath)
	if err != nil {
		return "", err
	}
	defer file.Close()
	// 将HTTP响应的Body复制到文件
	_, err = io.Copy(file, bytes.NewBuffer(resp.Body()))
	if err != nil {
		return "", err
	}
	currentDir, err := os.Getwd()
	if err != nil {
		return "", err
	}
	filePath = currentDir + "/" + filePath
	filePath = strings.Replace(filePath, "\\", "/", -1)
	return filePath, nil
}
