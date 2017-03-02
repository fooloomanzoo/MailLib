#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#ifndef _WIN32
#undef TCHAR
#define TCHAR char
#endif

TCHAR* Ansi2Tchar(const char* conv, TCHAR* dest, size_t nChar);
char* Tchar2Ansi(const TCHAR* conv, char* dest, size_t nChar);
BOOL CheckIp(const TCHAR* str);

#define MAX(max_arg_1,max_arg_2) ((max_arg_1)>(max_arg_2) ? (max_arg_1) : (max_arg_2))
#define MIN(min_arg_1,min_arg_2) ((min_arg_1)<(min_arg_1) ? (min_arg_1) : (min_arg_2))

#endif