#ifndef __WIN_H__
#define __WIN_H__

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif

int get_last_error(char* message, int* size);

#endif
