#include <map>
#include <string>

#include "exec_sql.h"
#include "load_calls.h"

using namespace std;

extern WxCalls_t g_WxCalls;
extern DWORD g_WeChatWinDllAddr;

typedef map<wstring, DWORD> dbMap_t;
static dbMap_t dbMap;

vector<wstring> GetDbNames()
{
    vector<wstring> vDbs;
    if (dbMap.empty()) {
        DWORD sqlHandleBaseAddr  = *(DWORD *)(g_WeChatWinDllAddr + g_WxCalls.sql.base);
        DWORD sqlHandleBeginAddr = *(DWORD *)(sqlHandleBaseAddr + g_WxCalls.sql.start);
        DWORD sqlHandleEndAddr   = *(DWORD *)(sqlHandleBaseAddr + g_WxCalls.sql.end);
        while (sqlHandleBeginAddr < sqlHandleEndAddr) {
            DWORD dwHandle = *(DWORD *)sqlHandleBeginAddr;
            dbMap[wstring((wchar_t *)(*(DWORD *)(dwHandle + g_WxCalls.sql.name)))]
                = *(DWORD *)(dwHandle + g_WxCalls.sql.slot);
            sqlHandleBeginAddr += 0x04;
        }
    }
    for (auto it = dbMap.begin(); it != dbMap.end(); it++) {
        vDbs.push_back(it->first);
    }
    return vDbs;
}
