#pragma once

#include <stdint.h>
#include <string>
#include <vector>

typedef struct {
    double filetime;
    int32_t totPackets;
} DecTime_t;

int Mp3Encode(std::vector<uint8_t> &pcm, std::string &mp3path, int32_t sr);
int Mp3Encode(std::vector<uint8_t> &pcm, std::vector<uint8_t> &mp3, int32_t sr);
DecTime_t SilkDecode(std::vector<uint8_t> &silk, std::vector<uint8_t> &pcm, int32_t sr);

int Silk2Mp3(std::string inpath, std::string outpath, int sr);
int Silk2Mp3(std::vector<uint8_t> &silk, std::string mp3path, int sr);
int Silk2Mp3(std::vector<uint8_t> &silk, std::vector<uint8_t> &mp3, int sr);
