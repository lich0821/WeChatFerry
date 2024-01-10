package wcfrest

import (
	"errors"

	"github.com/opentdp/go-helper/logman"
	"github.com/opentdp/go-helper/request"

	"github.com/opentdp/wechat-rest/wcferry"
)

var forwardToUrlStat = false
var forwardToUrlList = map[string]bool{}

func (wc *Controller) enableForwardToUrl(url string) error {

	if !forwardToUrlStat {
		err := wc.EnrollReceiver(true, func(msg *wcferry.WxMsg) {
			ret := wcferry.WxMsgParser(msg)
			for url := range forwardToUrlList {
				logman.Info("forward msg", "url", url, "Id", ret.Id)
				go request.JsonPost(url, ret, request.H{})
			}
		})
		if err != nil {
			return err
		}
	}

	if _, ok := forwardToUrlList[url]; ok {
		return errors.New("url already exists")
	}

	forwardToUrlStat = true
	forwardToUrlList[url] = true

	return nil

}

func (wc *Controller) disableForwardToUrl(url string) error {

	if _, ok := forwardToUrlList[url]; !ok {
		return errors.New("url not exists")
	}

	delete(forwardToUrlList, url)

	if len(forwardToUrlList) == 0 {
		if err := wc.DisableReceiver(false); err != nil {
			return err
		}
		forwardToUrlStat = false
	}

	return nil

}
