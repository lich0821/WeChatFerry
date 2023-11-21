#pragma once

#include "stdint.h"
#include <string>

bool DecryptImage(std::string src, std::string dst);
int RefreshPyq(uint64_t id);
std::string DownloadAttach(uint64_t id, std::string thumb, std::string extra);
