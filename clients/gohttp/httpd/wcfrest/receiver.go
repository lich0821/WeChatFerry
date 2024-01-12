package wcfrest

import (
	"errors"

	"github.com/opentdp/go-helper/logman"
	"github.com/opentdp/go-helper/request"

	"github.com/opentdp/wechat-rest/wcferry"
)

var urlReceiverStat = false
var urlReceiverList = map[string]bool{}

func (wc *Controller) enableUrlReceiver(url string) error {

	if !urlReceiverStat {
		err := wc.EnrollReceiver(true, func(msg *wcferry.WxMsg) {
			ret := wcferry.ParseWxMsg(msg)
			for url := range urlReceiverList {
				logman.Info("forward msg", "url", url, "Id", ret.Id)
				go request.JsonPost(url, ret, request.H{})
			}
		})
		if err != nil {
			return err
		}
	}

	if _, ok := urlReceiverList[url]; ok {
		return errors.New("url already exists")
	}

	urlReceiverStat = true
	urlReceiverList[url] = true

	return nil

}

func (wc *Controller) disableUrlReceiver(url string) error {

	if _, ok := urlReceiverList[url]; !ok {
		return errors.New("url not exists")
	}

	delete(urlReceiverList, url)

	if len(urlReceiverList) == 0 {
		if err := wc.DisableReceiver(false); err != nil {
			return err
		}
		urlReceiverStat = false
	}

	return nil

}
