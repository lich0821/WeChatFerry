#pragma once

#include "stdint.h"
#include <string>

std::string GetAudio(uint64_t id, std::string dir);
std::string DecryptImage(std::string src, std::string dst);
int RefreshPyq(uint64_t id);
int DownloadAttach(uint64_t id, std::string thumb, std::string extra);
int RevokeMsg(uint64_t id);
OcrResult_t GetOcrResult(std::string path);
