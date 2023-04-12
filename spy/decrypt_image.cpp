#include <fstream>

#include "decrypt_image.h"
#include "log.h"

#define HEADER_PNG1 0x89
#define HEADER_PNG2 0x50
#define HEADER_JPG1 0xFF
#define HEADER_JPG2 0xD8
#define HEADER_GIF1 0x47
#define HEADER_GIF2 0x49

using namespace std;

static string get_key(uint8_t header1, uint8_t header2, uint8_t *key)
{
    // PNG?
    *key = HEADER_PNG1 ^ header1;
    if ((HEADER_PNG2 ^ *key) == header2) {
        return ".png";
    }

    // JPG?
    *key = HEADER_JPG1 ^ header1;
    if ((HEADER_JPG2 ^ *key) == header2) {
        return ".jpg";
    }

    // GIF?
    *key = HEADER_GIF1 ^ header1;
    if ((HEADER_GIF2 ^ *key) == header2) {
        return ".gif";
    }

    return ""; // 错误
}

bool DecryptImage(string src, string dst)
{
    ifstream in(src.c_str(), ios::binary);
    if (!in.is_open()) {
        LOG_ERROR("Failed to open file {}", src);
        return false;
    }

    filebuf *pfb = in.rdbuf();
    size_t size  = pfb->pubseekoff(0, ios::end, ios::in);
    pfb->pubseekpos(0, ios::in);

    char *pBuf = new char[size];
    pfb->sgetn(pBuf, size);
    in.close();

    uint8_t key = 0x00;
    string ext  = get_key(pBuf[0], pBuf[1], &key);
    if (ext.empty()) {
        LOG_ERROR("Failed to get key.");
        return false;
    }

    for (size_t i = 0; i < size; i++) {
        pBuf[i] ^= key;
    }

    ofstream out((dst + ext).c_str(), ios::binary);
    if (!out.is_open()) {
        LOG_ERROR("Failed to open file {}", dst);
        return false;
    }

    out.write(pBuf, size);
    out.close();

    delete[] pBuf;

    return true;
}
