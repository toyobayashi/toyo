#ifndef __WIN_H__
#define __WIN_H__

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

int get_last_error(char* message, int* size);

#ifdef __cplusplus
}
#endif

#endif
