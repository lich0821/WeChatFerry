#pragma once

#include "stdint.h"
#include <string>

string DecryptImage(std::string src, std::string dst);
int RefreshPyq(uint64_t id);
int DownloadAttach(uint64_t id, std::string thumb, std::string extra);
