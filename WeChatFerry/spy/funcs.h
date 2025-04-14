#pragma once

#include "stdint.h"
#include <string>

int IsLogin(void);
std::string GetAudio(uint64_t id, std::string dir);
std::string GetPCMAudio(uint64_t id, std::string dir, int32_t sr);
std::string DecryptImage(std::string src, std::string dst);
int RefreshPyq(uint64_t id);
int DownloadAttach(uint64_t id, std::string thumb, std::string extra);
int RevokeMsg(uint64_t id);
OcrResult_t GetOcrResult(std::string path);
string GetLoginUrl();
int ReceiveTransfer(std::string wxid, std::string transferid, std::string transactionid);
