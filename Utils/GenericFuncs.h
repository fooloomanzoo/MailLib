#ifndef GENERIC_FUNCS_H_INCLUDED
#define GENERIC_FUNCS_H_INCLUDED

#ifdef _WIN32
typedef SOCKET generic_sd_t;
#define generic_invalid_socket INVALID_SOCKET
#else
typedef int generic_sd_t;
#define generic_invalid_socket -1
#endif

typedef enum{false,true} bool;

bool generic_socket_init(void);
bool generic_socket_finish(void);
generic_sd_t generic_stream_socket_create(void);
bool generic_socket_close(generic_sd_t sd);
bool generic_socket_connect(generic_sd_t* sd, const char* szIp, unsigned short nPort);
size_t generic_socket_recv(generic_sd_t sd, unsigned char* pBuf, size_t nBufSize, unsigned const char* szDelim, unsigned long nTimeOut, unsigned long* pnPastTime);
bool generic_socket_send(generic_sd_t sd, const unsigned char* pBuf, size_t nBufSize);
bool generic_gethostbyname(const char* szHostName, char szIp[16]);

void* generic_malloc(size_t size);
void* generic_realloc(void* pBlock, size_t size);
void generic_free(void* pBlock);

int generic_snprintf(char* pStrBuf, size_t nBufSize, const char* szFormat, ...);
char* generic_strstri(char* szFirst, const char* szSrch);
int generic_strncmpi(const char *sz1, const char *sz2, size_t count);

#endif
