#pragma warning(disable : 4244)

#include "framework.h"
#include <filesystem>
#include <fstream>
#include <io.h>
#include <direct.h>

#include "codec.h"
#include "exec_sql.h"
#include "funcs.h"
#include "log.h"
#include "spy_types.h"
#include "util.h"

using namespace std;
namespace fs = std::filesystem;

extern bool gIsListeningPyq;
extern QWORD g_WeChatWinDllAddr;

#define HEADER_PNG1 0x89
#define HEADER_PNG2 0x50
#define HEADER_JPG1 0xFF
#define HEADER_JPG2 0xD8
#define HEADER_GIF1 0x47
#define HEADER_GIF2 0x49

#define OS_LOGIN_STATUS               0x595C9E8
#define OS_GET_SNS_DATA_MGR           0x21E2200
#define OS_GET_SNS_FIRST_PAGE         0x2E212D0
#define OS_GET_SNS_TIMELINE_MGR       0x2DB3390
#define OS_GET_SNS_NEXT_PAGE          0x2EC8970
#define OS_NEW_CHAT_MSG               0x1B5E140
#define OS_FREE_CHAT_MSG              0x1B55850
#define OS_GET_CHAT_MGR               0x1B876C0
#define OS_GET_MGR_BY_PREFIX_LOCAL_ID 0x213FB00
#define OS_GET_PRE_DOWNLOAD_MGR       0x1C0EE70
#define OS_PUSH_ATTACH_TASK           0x1CDF4E0
#define OS_LOGIN_QR_CODE              0x59620D8

typedef QWORD (*GetSNSDataMgr_t)();
typedef QWORD (*GetSnsTimeLineMgr_t)();
typedef QWORD (*GetSNSFirstPage_t)(QWORD, QWORD, QWORD);
typedef QWORD (*GetSNSNextPageScene_t)(QWORD, QWORD);
typedef QWORD (*GetChatMgr_t)();
typedef QWORD (*NewChatMsg_t)(QWORD);
typedef QWORD (*FreeChatMsg_t)(QWORD);
typedef QWORD (*GetPreDownLoadMgr_t)();
typedef QWORD (*GetMgrByPrefixLocalId_t)(QWORD, QWORD);
typedef QWORD (*PushAttachTask_t)(QWORD, QWORD, QWORD, QWORD);
typedef QWORD (*GetOCRManager_t)();
typedef QWORD (*DoOCRTask_t)(QWORD, QWORD, QWORD, QWORD, QWORD, QWORD);

int IsLogin(void) { return (int)GET_QWORD(g_WeChatWinDllAddr + OS_LOGIN_STATUS); }

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

// 创建多级目录
bool CreateDir(const char* dir)
{
    int m = 0, n;
    string str1, str2;
    str1 = dir;
    str2 = str1.substr(0, 2);
    str1 = str1.substr(3, str1.size());
    while (m >= 0)
    {
        m = str1.find('/');

        str2 += '/' + str1.substr(0, m);
        //判断该目录是否存在
        n = _access(str2.c_str(), 0);
        if (n == -1)
        {
            //创建目录文件
            int flag = _mkdir(str2.c_str());
            if (flag != 0) { //创建失败
                LOG_ERROR("Failed to CreateDir:{}", dir);
                return false;
            }
        }

        str1 = str1.substr(m + 1, str1.size());
    }
    LOG_DEBUG("CreateDir {} success.", dir);
    return true;
}

string DecryptImage(string src, string dir)
{
    if (!fs::exists(src)) {
        LOG_ERROR("File not exists: {}", src);
        return "";
    }

    ifstream in(src.c_str(), ios::binary);
    if (!in.is_open()) {
        LOG_ERROR("Failed to read file {}", src);
        return "";
    }

    filebuf *pfb = in.rdbuf();
    size_t size  = pfb->pubseekoff(0, ios::end, ios::in);
    pfb->pubseekpos(0, ios::in);

    vector<char> buff;
    buff.reserve(size);
    char *pBuf = buff.data();
    pfb->sgetn(pBuf, size);
    in.close();

    uint8_t key = 0x00;
    string ext  = get_key(pBuf[0], pBuf[1], &key);
    if (ext.empty()) {
        LOG_ERROR("Failed to get key.");
        return "";
    }

    for (size_t i = 0; i < size; i++) {
        pBuf[i] ^= key;
    }

    string dst = "";

    try {
        if (dir.empty()) {
            dst = fs::path(src).replace_extension(ext).string();
        } else {
            dst = (dir.back() == '\\' || dir.back() == '/') ? dir : (dir + "/");
            replace(dst.begin(), dst.end(), '\\', '/');

            // 判断dir文件夹是否存在，若不存在则创建（否则将无法创建出文件）
            if (_access(dst.c_str(), 0) == -1) {//判断该文件夹是否存在
                bool success = CreateDir(dst.c_str()); //Windows创建文件夹
                if (!success) { //创建失败
                    LOG_ERROR("Failed to mkdir:{}", dst);
                    return "";
                }
            }
            
            dst += fs::path(src).stem().string() + ext;
        }

        replace(dst.begin(), dst.end(), '\\', '/');
    } catch (const std::exception &e) {
        LOG_ERROR(GB2312ToUtf8(e.what()));
    } catch (...) {
        LOG_ERROR("Unknow exception.");
        return "";
    }

    ofstream out(dst.c_str(), ios::binary);
    if (!out.is_open()) {
        LOG_ERROR("Failed to write file {}", dst);
        return "";
    }

    out.write(pBuf, size);
    out.close();

    return dst;
}

static int GetFirstPage()
{
    int status = -1;

    GetSNSDataMgr_t GetSNSDataMgr     = (GetSNSDataMgr_t)(g_WeChatWinDllAddr + OS_GET_SNS_DATA_MGR);
    GetSNSFirstPage_t GetSNSFirstPage = (GetSNSFirstPage_t)(g_WeChatWinDllAddr + OS_GET_SNS_FIRST_PAGE);

    QWORD buff[16] = { 0 };
    QWORD mgr      = GetSNSDataMgr();
    status         = (int)GetSNSFirstPage(mgr, (QWORD)buff, 1);

    return status;
}

static int GetNextPage(QWORD id)
{
    int status = -1;

    GetSnsTimeLineMgr_t GetSnsTimeLineMgr     = (GetSnsTimeLineMgr_t)(g_WeChatWinDllAddr + OS_GET_SNS_TIMELINE_MGR);
    GetSNSNextPageScene_t GetSNSNextPageScene = (GetSNSNextPageScene_t)(g_WeChatWinDllAddr + OS_GET_SNS_NEXT_PAGE);

    QWORD mgr = GetSnsTimeLineMgr();
    status    = (int)GetSNSNextPageScene(mgr, id);

    return status;
}

int RefreshPyq(QWORD id)
{
    if (!gIsListeningPyq) {
        LOG_ERROR("没有启动朋友圈消息接收，参考：enable_receiving_msg");
        return -1;
    }

    if (id == 0) {
        return GetFirstPage();
    }

    return GetNextPage(id);
}

/*******************************************************************************
 * 都说我不写注释，写一下吧
 * 其实也没啥好写的，就是下载资源
 * 主要介绍一下几个参数：
 * id：好理解，消息 id
 * thumb：图片或者视频的缩略图路径；如果是视频，后缀为 mp4 后就是存在路径了
 * extra：图片、文件的路径
 *******************************************************************************/
int DownloadAttach(QWORD id, string thumb, string extra)
{
    int status = -1;
    QWORD localId;
    uint32_t dbIdx;

    if (fs::exists(extra)) { // 第一道，不重复下载。TODO: 通过文件大小来判断
        return 0;
    }

    if (GetLocalIdandDbidx(id, &localId, &dbIdx) != 0) {
        LOG_ERROR("Failed to get localId, Please check id: {}", to_string(id));
        return status;
    }

    NewChatMsg_t NewChatMsg               = (NewChatMsg_t)(g_WeChatWinDllAddr + OS_NEW_CHAT_MSG);
    FreeChatMsg_t FreeChatMsg             = (FreeChatMsg_t)(g_WeChatWinDllAddr + OS_FREE_CHAT_MSG);
    GetChatMgr_t GetChatMgr               = (GetChatMgr_t)(g_WeChatWinDllAddr + OS_GET_CHAT_MGR);
    GetPreDownLoadMgr_t GetPreDownLoadMgr = (GetPreDownLoadMgr_t)(g_WeChatWinDllAddr + OS_GET_PRE_DOWNLOAD_MGR);
    PushAttachTask_t PushAttachTask       = (PushAttachTask_t)(g_WeChatWinDllAddr + OS_PUSH_ATTACH_TASK);
    GetMgrByPrefixLocalId_t GetMgrByPrefixLocalId
        = (GetMgrByPrefixLocalId_t)(g_WeChatWinDllAddr + OS_GET_MGR_BY_PREFIX_LOCAL_ID);

    LARGE_INTEGER l;
    l.HighPart = dbIdx;
    l.LowPart  = (DWORD)localId;

    char *buff = (char *)HeapAlloc(GetProcessHeap(), 0, 0x460);
    if (buff == nullptr) {
        LOG_ERROR("Failed to allocate memory.");
        return status;
    }

    QWORD pChatMsg = NewChatMsg((QWORD)buff);
    GetChatMgr();
    GetMgrByPrefixLocalId(l.QuadPart, pChatMsg);

    QWORD type = GET_QWORD(buff + 0x38);

    string save_path  = "";
    string thumb_path = "";

    switch (type) {
        case 0x03: { // Image: extra
            save_path = extra;
            break;
        }
        case 0x3E:
        case 0x2B: { // Video: thumb
            thumb_path = thumb;
            save_path  = fs::path(thumb).replace_extension("mp4").string();
            break;
        }
        case 0x31: { // File: extra
            save_path = extra;
            break;
        }
        default:
            break;
    }

    if (fs::exists(save_path)) { // 不重复下载。TODO: 通过文件大小来判断
        return 0;
    }

    LOG_DEBUG("path: {}", save_path);
    // 创建父目录，由于路径来源于微信，不做检查
    fs::create_directory(fs::path(save_path).parent_path().string());

    int temp             = 1;
    WxString *pSavePath  = NewWxStringFromStr(save_path);
    WxString *pThumbPath = NewWxStringFromStr(thumb_path);

    memcpy(&buff[0x280], pThumbPath, sizeof(WxString));
    memcpy(&buff[0x2A0], pSavePath, sizeof(WxString));
    memcpy(&buff[0x40C], &temp, sizeof(temp));

    QWORD mgr = GetPreDownLoadMgr();
    status    = (int)PushAttachTask(mgr, pChatMsg, 0, 1);
    FreeChatMsg(pChatMsg);

    return status;
}

string GetAudio(QWORD id, string dir)
{
    string mp3path = (dir.back() == '\\' || dir.back() == '/') ? dir : (dir + "/");
    mp3path += to_string(id) + ".mp3";
    replace(mp3path.begin(), mp3path.end(), '\\', '/');
    if (fs::exists(mp3path)) { // 不重复下载
        return mp3path;
    }

    vector<uint8_t> silk = GetAudioData(id);
    if (silk.size() == 0) {
        LOG_ERROR("Empty audio data.");
        return "";
    }

    Silk2Mp3(silk, mp3path, 24000);

    return mp3path;
}

string GetPCMAudio(uint64_t id, string dir, int32_t sr)
{
    string pcmpath = (dir.back() == '\\' || dir.back() == '/') ? dir : (dir + "/");
    pcmpath += to_string(id) + ".pcm";
    replace(pcmpath.begin(), pcmpath.end(), '\\', '/');
    if (fs::exists(pcmpath)) { // 不重复下载
        return pcmpath;
    }
    vector<uint8_t> pcm;
    vector<uint8_t> silk = GetAudioData(id);
    if (silk.size() == 0) {
        LOG_ERROR("Empty audio data.");
        return "";
    }

    SilkDecode(silk, pcm, sr);
    errno_t err;
    FILE* fPCM;
    err = fopen_s(&fPCM, pcmpath.c_str(), "wb");
    if (err != 0) {
        printf("Error: could not open input file %s\n", pcmpath.c_str());
        exit(0);
    }

    fwrite(pcm.data(), sizeof(uint8_t), pcm.size(), fPCM);
    fclose(fPCM);

    return pcmpath;
}


OcrResult_t GetOcrResult(string path)
{
    OcrResult_t ret = { -1, "" };
#if 0 // 参数没调好，会抛异常，看看有没有好心人来修复
    if (!fs::exists(path)) {
        LOG_ERROR("Can not find: {}", path);
        return ret;
    }

    GetOCRManager_t GetOCRManager = (GetOCRManager_t)(g_WeChatWinDllAddr + 0x1D6C3C0);
    DoOCRTask_t DoOCRTask         = (DoOCRTask_t)(g_WeChatWinDllAddr + 0x2D10BC0);

    QWORD unk1 = 0, unk2 = 0, unused = 0;
    QWORD *pUnk1 = &unk1;
    QWORD *pUnk2 = &unk2;
    // 路径分隔符有要求，必须为 `\`
    wstring wsPath = String2Wstring(fs::path(path).make_preferred().string());
    WxString wxPath(wsPath);
    vector<QWORD> *pv = (vector<QWORD> *)HeapAlloc(GetProcessHeap(), 0, 0x20);
    RawVector_t *pRv  = (RawVector_t *)pv;
    pRv->finish       = pRv->start;
    char buff[0x98]   = { 0 };
    memcpy(buff, &pRv->start, sizeof(QWORD));

    QWORD mgr  = GetOCRManager();
    ret.status = (int)DoOCRTask(mgr, (QWORD)&wxPath, unused, (QWORD)buff, (QWORD)&pUnk1, (QWORD)&pUnk2);

    QWORD count = GET_QWORD(buff + 0x8);
    if (count > 0) {
        QWORD header = GET_QWORD(buff);
        for (QWORD i = 0; i < count; i++) {
            QWORD content = GET_QWORD(header);
            ret.result += Wstring2String(GET_WSTRING(content + 0x28));
            ret.result += "\n";
            header = content;
        }
    }
#endif
    return ret;
}

int RevokeMsg(QWORD id)
{
    int status = -1;
#if 0 // 这个挺鸡肋的，因为自己发的消息没法直接获得 msgid，就这样吧
    QWORD localId;
    uint32_t dbIdx;
    if (GetLocalIdandDbidx(id, &localId, &dbIdx) != 0) {
        LOG_ERROR("Failed to get localId, Please check id: {}", to_string(id));
        return status;
    }
#endif
    return status;
}

string GetLoginUrl()
{
    LPVOID targetAddress = reinterpret_cast<LPBYTE>(g_WeChatWinDllAddr) + OS_LOGIN_QR_CODE;

    char *dataPtr = *reinterpret_cast<char **>(targetAddress); // 读取指针内容
    if (!dataPtr) {
        LOG_ERROR("Failed to get login url");
        return "error";
    }

    // 读取字符串内容
    std::string data(dataPtr, 22);
    return "http://weixin.qq.com/x/" + data;
}

int ReceiveTransfer(string wxid, string transferid, string transactionid)
{
    // 别想了，这个不实现了
    return -1;
}
