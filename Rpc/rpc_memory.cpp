#include <windows.h>

/******************************************************/
/*         MIDL allocate and free                     */
/******************************************************/

void __RPC_FAR *__RPC_USER midl_user_allocate(size_t len) { return (malloc(len)); }

void __RPC_USER midl_user_free(void __RPC_FAR *ptr) { free(ptr); }
