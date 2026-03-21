# WeChatFerry-go

[WeChatFerry](https://github.com/lich0821/WeChatFerry) golang client

## Usage

```go
package main

import (
	"github.com/danbai225/WeChatFerry-go/wcf"
)

func main() {
	c, err := wcf.NewWCF("")
	if err != nil {
		panic(err)
	}
	println(c.IsLogin())
}

```
