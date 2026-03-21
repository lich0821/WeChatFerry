@echo off
chcp 65001 > nul
cd /d %~dp0

if "%CI%" == "true" (
    echo 自动化编译环境，跳过手动确认。
    echo 用户已接受开源协议 > license_accepted.flag
    exit /b 0
)

:: 检查是否已接受协议
if exist ".license_accepted.flag" (
    echo 用户已接受免责声明
    exit /b 0
)

:: 检查免责声明文件是否存在
if not exist "DISCLAIMER.md" (
    echo 错误：未找到免责声明文件 DISCLAIMER.md。
    exit /b 1
)

:: 生成临时文件并读取免责声明
type "DISCLAIMER.md" > temp_disclaimer.txt

:: 使用 PowerShell 显示弹窗
powershell -NoProfile -Command ^
    "$text = [System.IO.File]::ReadAllText('temp_disclaimer.txt', [System.Text.Encoding]::UTF8);" ^
    "Add-Type -AssemblyName PresentationFramework;" ^
    "$result = [System.Windows.MessageBox]::Show($text, '免责声明', 'OKCancel', 'Warning');" ^
    "if ($result -ne 'OK') { exit 1 }"

:: 检查 PowerShell 执行结果
if %errorlevel% neq 0 (
    echo 错误：用户未接受免责声明。停止生成。
    del temp_disclaimer.txt
    exit /b 1
)

:: 用户接受免责声明，创建标志文件
echo 用户已接受免责声明 > .license_accepted.flag
del temp_disclaimer.txt
echo 用户已接受免责声明
exit /b 0
