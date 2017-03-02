#ifdef _WIN32
#include <windows.h>
#else
#include <clib.h>
#endif

#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include "GenericFuncs.h"

bool generic_socket_init(void)
{
#ifdef _WIN32
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 0), &WSAData) != 0) /*mindestens WINSOCK v2.0*/
		return false;
#endif

	return true;
}

bool generic_socket_finish(void)
{
#ifdef _WIN32
	WSACleanup();
#endif

	return true;
}

generic_sd_t generic_stream_socket_create(void)
{
#ifdef _WIN32
	return socket(AF_INET, SOCK_STREAM, 0);
#else
	int error;
	return opensocket(SOCK_STREAM, &error);
#endif
}

bool generic_socket_close(generic_sd_t sd)
{
#ifdef _WIN32
	return closesocket(sd) == 0;
#else
	int error;
	return closesocket(sd, &error) == 0;
#endif
}

bool generic_socket_connect(generic_sd_t* sd, const char* szIp, unsigned short nPort)
{
	int rc;

#ifdef _WIN32
	SOCKADDR_IN sa;

	if (szIp == NULL)
		return false;
	memset(&sa, 0, sizeof(SOCKADDR_IN));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(nPort);
	sa.sin_addr.s_addr = inet_addr(szIp);
	rc = connect(*sd, (SOCKADDR*)&sa, sizeof(sa));
	if (rc != 0)
	{
		generic_socket_close(*sd);
		*sd = generic_invalid_socket;
	}
	return rc == 0;

#else
	struct sockaddr_in sa;
	int error;

	if (szIp == NULL)
		return false;
	sa.sin_family = PF_INET;
	sa.sin_port = htons(nPort);
	if (inet_addr(szIp, &sa.sin_addr.s_addr) != 0)
		return 0;
	rc = connect(*sd, (const struct sockaddr far*)&sa, &error);
	if (rc != 0)
	{
		generic_socket_close(*sd);
		*sd = generic_invalid_socket;
	}
	return rc == 0;
#endif
}

#define TIME_OUT_ON_SOCKET //über setsocketopt wurde Timeout für Empfangen setzt
size_t generic_socket_recv(generic_sd_t sd, unsigned char* pBuf, size_t nBufSize, unsigned const char* szDelim, unsigned long nTimeOut, unsigned long* pnPastTime)
{
	size_t nBytesRead = 0;
	unsigned long nTimeLeft = nTimeOut, nPastTime = 0;
	time_t t0;

#ifdef _WIN32
	int res;
#ifndef TIME_OUT_ON_SOCKET
	unsigned long nPendingBytes;
#endif

	if (sd == generic_invalid_socket || pBuf == NULL || nBufSize < 1)
		return false;

	if (pnPastTime)
		*pnPastTime = 0;

	do
	{
		t0 = clock();

#ifdef TIME_OUT_ON_SOCKET
		if ((res = recv(sd, pBuf + nBytesRead, nBufSize - nBytesRead, 0)) != SOCKET_ERROR)
			nBytesRead += res;
#else
		if (ioctlsocket(sd, FIONREAD, &nPendingBytes) == 0)
			if (nPendingBytes > 0)
				if ((res = recv(sd, pBuf + nBytesRead, nBufSize - nBytesRead, 0)) != SOCKET_ERROR)
					nBytesRead += res;
#endif

		nPastTime += (unsigned long)((((double)(clock() - t0)) / CLOCKS_PER_SEC)*1000.0);

		if (res != SOCKET_ERROR && szDelim)
			if (strstr(pBuf, szDelim))
				break;

		if (nPastTime > nTimeLeft)
			nTimeLeft = 0;
		else
			nTimeLeft -= nPastTime;
	} while (nBytesRead < nBufSize && nTimeLeft>0);

#else
	int error, res;
	unsigned long nFac;

	if (sd == generic_invalid_socket || pBuf == NULL || nBufSize < 1)
		return false;

	nFac = 1000L / CLOCKS_PER_SEC;
	if (nFac == 0)
		++nFac;
	do
	{
		t0 = clock();
		if ((res = recv(sd, pBuf + nBytesRead, nBufSize - nBytesRead, MSG_TIMEOUT, nTimeLeft, &error)) > 0)
			nBytesRead += res;

		nPastTime += (clock() - t0)*nFac;

		if (res > 0 && szDelim)
			if (strstr(pBuf, szDelim))
				break;

		if (nPastTime > nTimeLeft)
			nTimeLeft = 0;
		else
			nTimeLeft -= nPastTime;
	} while (nBytesRead < nBufSize && nTimeLeft>0);
#endif

	if (pnPastTime)
		*pnPastTime = nPastTime;
	return nBytesRead;
}

bool generic_socket_send(generic_sd_t sd, const unsigned char* pBuf, size_t nBufSize)
{
#ifdef _WIN32
	return send(sd, pBuf, nBufSize, 0) != SOCKET_ERROR;

#else
	int error;
	return send(sd, pBuf, nBufSize, MSG_BLOCKING, &error) == 0;
#endif
}

bool generic_gethostbyname(const char* szHostName, char szIp[16])
{
#ifdef _WIN32
	HOSTENT* remoteHost = NULL;

	if (szHostName == NULL || szIp == NULL)
		return false;

	remoteHost = gethostbyname(szHostName);
	if (remoteHost == NULL)
		return false;
	strncpy(szIp, inet_ntoa(*(struct in_addr*)remoteHost->h_addr_list[0]), 16);

#else
	if (szHostName == NULL || szIp == NULL)
		return false;

	return false;
#endif


	return true;
}

#ifndef _WIN32
#include <alloc.h>
#endif
void* generic_malloc(size_t size)
{
#ifdef _WIN32
	return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);

#else
	return farmalloc(size);
#endif
}

void* generic_realloc(void* pBlock, size_t size)
{
#ifdef _WIN32
	if (pBlock == NULL)
		return generic_malloc(size);
	else
		return HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, pBlock, size);

#else
	return farrealloc(pBlock, size);
#endif
}

void generic_free(void* pBlock)
{
#ifdef _WIN32
	HeapFree(GetProcessHeap(), 0, pBlock);

#else
	farfree(pBlock);
#endif
}

int generic_snprintf(char* pStrBuf, size_t nBufSize, const char* szFormat, ...)
{
	va_list argptr;
	int nChars;

	if (pStrBuf == NULL || nBufSize == 0 || szFormat == NULL)
		return 0;

	va_start(argptr, szFormat);

#ifdef _WIN32
	nChars = _vsnprintf(pStrBuf, nBufSize, szFormat, argptr);
#else
	nChars = vsprintf(pStrBuf, szFormat, argptr);
#endif

	va_end(argptr);
	pStrBuf[nBufSize - 1] = '\0';
	return nChars;
}

#ifdef _WIN32
#include <shlwapi.h>
#endif
char* generic_strstri(char* szFirst, const char* szSrch)
{
#ifdef _WIN32
	return StrStrIA(szFirst, szSrch);

#else
	size_t ncFirst, ncSrch, i;

	if (szFirst == NULL || szSrch == NULL)
		return NULL;

	ncFirst = strlen(szFirst);
	ncSrch = strlen(szSrch);
	if (ncSrch == 0)
		return szFirst;
	for (i = 0; i < ncFirst - ncSrch + 1; ++i)
	{
		if (strncmpi(szFirst + i, szSrch, ncStrch) == 0)
			return szFirst + i;
	}
	return NULL;
#endif
}

int generic_strncmpi(const char *sz1, const char *sz2, size_t count)
{
	if (sz1 == NULL || sz2 == NULL)
		return -123;
	if (count == 0)
		return 0;

#ifdef _WIN32
	return _strnicmp(sz1, sz2, count);
#else
	return strncmpi(sz1, sz2, count);
#endif
}
