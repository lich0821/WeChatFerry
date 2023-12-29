package wcfrest

import (
	"errors"

	"github.com/opentdp/go-helper/logman"
	"github.com/opentdp/go-helper/request"

	"github.com/opentdp/wechat-rest/wcferry"
)

var forwardToUrlStat = false
var forwardToUrlList = map[string]bool{}

func enableForwardToUrl(url string) error {

	if !forwardToUrlStat {
		err := wc.EnrollReceiver(true, func(msg *wcferry.WxMsg) {
			for url := range forwardToUrlList {
				logman.Info("forward msg", "url", url, "Id", msg.Id)
				request.JsonPost(url, msg, request.H{})
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

func disableForwardToUrl(url string) error {

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
