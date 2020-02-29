#ifdef _WIN32

#include "winerr.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "windlfcn.h"
#include "charset.hpp"

/**
 * Win32 error code from last failure.
 */

static DWORD lastError = 0;

/**
 * Open DLL, returning a handle.
 */
void* dlopen(const char* file, int mode) {
  UINT errorMode;
  void* handle;

  UNREFERENCED_PARAMETER(mode);

  if (file == NULL) return (void*) GetModuleHandle(NULL);

  errorMode = GetErrorMode();

  /* Have LoadLibrary return NULL on failure; prevent GUI error message. */
  SetErrorMode(errorMode | SEM_FAILCRITICALERRORS);

  handle = (void*) LoadLibraryW(toyo::charset::a2w(file).c_str());

  if (handle == NULL) lastError = GetLastError();

  SetErrorMode(errorMode);

  return handle;
}

/**
 * Close DLL.
 */
int dlclose(void* handle) {
  int rc = 0;

  if (handle != (void*) GetModuleHandle(NULL))
    rc = !FreeLibrary((HMODULE) handle);

  if (rc)
    lastError = GetLastError();

  return rc;
}

/**
 * Look up symbol exported by DLL.
 */
void* dlsym(void* handle, const char* name) {
  void* address = (void*) GetProcAddress((HMODULE) handle, name);

  if (address == NULL)
    lastError = GetLastError();

  return address;
}

/**
 * Return message describing last error.
 */
char* dlerror() {
  static char errorMessage[260];
  memset(errorMessage, 0, sizeof(errorMessage));

  if (lastError != 0) {
    std::string errmsg = get_win32_error_message(lastError);
    strcpy(errorMessage, errmsg.c_str());
    lastError = 0;
    return errorMessage;
  } else {
    return NULL;
  }
}

#endif
