package wcfrest

import (
	"errors"

	"github.com/opentdp/go-helper/logman"
	"github.com/opentdp/go-helper/request"
	"golang.org/x/net/websocket"

	"github.com/opentdp/wechat-rest/wcferry"
)

var urlReceiverKey = ""
var urlReceiverList = map[string]bool{}

var socketReceiverKey = ""
var socketReceiverList = map[*websocket.Conn]bool{}

func (wc *Controller) enableUrlReceiver(url string) error {

	logman.Info("enable receiver", "url", url)

	if urlReceiverKey == "" {
		key, err := wc.EnrollReceiver(true, func(msg *wcferry.WxMsg) {
			ret := wcferry.ParseWxMsg(msg)
			for u := range urlReceiverList {
				logman.Info("call receiver", "url", u, "Id", ret.Id)
				go request.JsonPost(u, ret, request.H{})
			}
		})
		if err != nil {
			return err
		}
		urlReceiverKey = key
	}

	if _, ok := urlReceiverList[url]; ok {
		return errors.New("url already exists")
	}

	urlReceiverList[url] = true
	return nil

}

func (wc *Controller) disableUrlReceiver(url string) error {

	logman.Info("disable receiver", "url", url)

	if _, ok := urlReceiverList[url]; !ok {
		return errors.New("url not exists")
	}

	delete(urlReceiverList, url)

	if len(urlReceiverList) == 0 {
		return wc.DisableReceiver(urlReceiverKey)
	}

	return nil

}

func (wc *Controller) enableSocketReceiver(ws *websocket.Conn) error {

	logman.Info("enable receiver", "socket", ws.RemoteAddr().String())

	if len(socketReceiverList) == 0 {
		key, err := wc.EnrollReceiver(true, func(msg *wcferry.WxMsg) {
			ret := wcferry.ParseWxMsg(msg)
			for w := range socketReceiverList {
				logman.Info("call receiver", "socket", ws.RemoteAddr().String(), "Id", ret.Id)
				go websocket.JSON.Send(w, ret)
			}
		})
		if err != nil {
			return err
		}
		socketReceiverKey = key
	}

	if _, ok := socketReceiverList[ws]; ok {
		return errors.New("socket already exists")
	}

	socketReceiverList[ws] = true
	return nil

}

func (wc *Controller) disableSocketReceiver(ws *websocket.Conn) error {

	logman.Info("disable receiver", "socket", ws.RemoteAddr().String())

	if _, ok := socketReceiverList[ws]; !ok {
		return errors.New("socket not exists")
	}

	delete(socketReceiverList, ws)

	if len(socketReceiverList) == 0 {
		return wc.DisableReceiver(socketReceiverKey)
	}

	return nil

}
