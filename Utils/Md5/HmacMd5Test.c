#include <windows.h>
#include <TCHAR.h>
#include <stdio.h>
#include <string.h>
#include "HmacMd5.h"

#define HMAC_TEST1
#define HMAC_TEST2
#define HMAC_TEST3
void DoHmacMd5Test(void)
{
	unsigned char rgDigest[16];

#ifdef HMAC_TEST1
	unsigned char rgKey1[16];
	unsigned char rgData1[9];
#endif
#ifdef HMAC_TEST2
	unsigned char rgKey2[5];
	unsigned char rgData2[29];
#endif
#ifdef HMAC_TEST3
	unsigned char rgKey3[16];
	unsigned char rgData3[50];
#endif
	int nDataLen, nKeyLen, i;

	TCHAR szOut[2*16+3];
	
#ifdef HMAC_TEST1
	lstrcpy(szOut,TEXT("0x"));
	nKeyLen = sizeof(rgKey1);
	memset(rgKey1,0x0b,nKeyLen);
	strcpy(rgData1,"Hi There");
	nDataLen = strlen(rgData1);
	hmac_md5(rgData1,nDataLen,rgKey1,nKeyLen,rgDigest);
	for(i=0; i<16; ++i)
		_sntprintf(szOut+lstrlen(szOut),sizeof(szOut)-lstrlen(szOut),TEXT("%02x"),rgDigest[i]);
	MessageBox(NULL,szOut,TEXT("Test1"),MB_ICONINFORMATION);
#endif

#ifdef HMAC_TEST2
	lstrcpy(szOut,TEXT("0x"));
	strcpy(rgKey2,"Jefe");
	nKeyLen = strlen(rgKey2);
	strcpy(rgData2,"what do ya want for nothing?");
	nDataLen = strlen(rgData2);
	hmac_md5(rgData2,nDataLen,rgKey2,nKeyLen,rgDigest);
	for(i=0; i<16; ++i)
		_sntprintf(szOut+lstrlen(szOut),sizeof(szOut)-lstrlen(szOut),TEXT("%02x"),rgDigest[i]);
	MessageBox(NULL,szOut,TEXT("Test2"),MB_ICONINFORMATION);
#endif

#ifdef HMAC_TEST3
	lstrcpy(szOut,TEXT("0x"));
	nKeyLen = sizeof(rgKey3);
	memset(rgKey3,0xAA,nKeyLen);
	nDataLen = sizeof(rgData3);
	memset(rgData3,0xDD,nDataLen);
	hmac_md5(rgData3,nDataLen,rgKey3,nKeyLen,rgDigest);
	for(i=0; i<16; ++i)
		_sntprintf(szOut+lstrlen(szOut),sizeof(szOut)-lstrlen(szOut),TEXT("%02x"),rgDigest[i]);
	MessageBox(NULL,szOut,TEXT("Test3"),MB_ICONINFORMATION);
#endif
}

