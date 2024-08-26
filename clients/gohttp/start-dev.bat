@ECHO OFF
::

SET CGO_ENABLED=0
SET GO111MODULE=on

SET GOOS=windows
SET GOARCH=amd64

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

CD /d %~dp0

go mod tidy

if exist .local.yml (
    echo use .local.yaml as config
    go run main.go .local.yml
) else (
    go run main.go
)
