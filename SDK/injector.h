#ifndef __INJECTOR_H__
#define __INJECTOR_H__

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#define INJERR_SUCCESS             0
#define INJERR_OTHER               -1
#define INJERR_NO_MEMORY           -2
#define INJERR_NO_PROCESS          -3
#define INJERR_NO_LIBRARY          -4
#define INJERR_NO_FUNCTION         -4
#define INJERR_ERROR_IN_TARGET     -5
#define INJERR_FILE_NOT_FOUND      -6
#define INJERR_INVALID_MEMORY_AREA -7
#define INJERR_PERMISSION          -8
#define INJERR_UNSUPPORTED_TARGET  -9
#define INJERR_INVALID_ELF_FORMAT  -10
#define INJERR_WAIT_TRACEE         -11

typedef struct injector injector_t;

int cki_attach(injector_t **injector, DWORD pid);
int cki_inject(injector_t *injector, const char *path, void **handle);
int cki_inject_w(injector_t *injector, const wchar_t *path, void **handle);
int cki_uninject(injector_t *injector, void *handle);
int cki_detach(injector_t *injector);
const char *cki_error(void);

BOOL InjectDll(DWORD pid, const WCHAR *dllpath);
BOOL EnjectDll(DWORD pid, const WCHAR *dllname);

#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif
