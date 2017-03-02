#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "Base64_0.h"

#define B64_TEST1
#define B64_TEST2
/*korrekte Testergebnisse: 
Test1: B64-String: "PDQwMzYuMTA3NjMyNDM4MUBndWlsZGVuc3Rlcm4uemVpdGZvcm0uZGU+"
Test2: B64-String: "YmVudXR6ZXJAemVpdGZvcm0uZGU="
*/
void DoBase64Tst(void)
{
	int ncB64, ncData, i;
	char szOut[999];

#ifdef B64_TEST1
	unsigned char rgOrgData1[] = "<4036.1076324381@guildenstern.zeitform.de>";
	int ncData1;
	char *szB64_1;
	unsigned char *rgDecData1;
#endif

#ifdef B64_TEST2
	unsigned char rgOrgData2[] = "benutzer@zeitform.de";
	int ncData2;
	char *szB64_2;
	unsigned char *rgDecData2;
#endif

#ifdef B64_TEST1
	ncData1 = strlen(rgOrgData1);
	ncB64 = B64Enc(&szB64_1,rgOrgData1,ncData1);
	if( ncB64<1 )
	{
		MessageBox(NULL,TEXT("Base64-Kodierung fehlgeschlagen!"),TEXT("Test1"),MB_ICONERROR);
		return;
	}
	ncData = B64Dec(&rgDecData1,szB64_1);
	if( ncData<1 )
	{
		MessageBox(NULL,TEXT("Base64-Dekodierung fehlgeschlagen!"),TEXT("Test1"),MB_ICONERROR);
		free(szB64_1);
		return;
	}

	_snprintf(szOut,sizeof(szOut),"Orginal-Daten (%d B):\t0x",ncData1);
	for(i=0; i<ncData1; ++i)
		_snprintf(szOut+strlen(szOut),sizeof(szOut)-strlen(szOut),"%02x",rgOrgData1[i]);
	_snprintf(szOut+strlen(szOut),sizeof(szOut)-strlen(szOut),"\nB64-String:\t%s",szB64_1);
	_snprintf(szOut+strlen(szOut),sizeof(szOut)-strlen(szOut),"\nDecodierte Daten (%d B):\t0x",ncData);
	for(i=0; i<ncData; ++i)
		_snprintf(szOut+strlen(szOut),sizeof(szOut)-strlen(szOut),"%02x",rgDecData1[i]);
	MessageBoxA(NULL,szOut,"Test1",MB_ICONINFORMATION);
	free(szB64_1);
	free(rgDecData1);
#endif

#ifdef B64_TEST2
	ncData2 = strlen(rgOrgData2);
	ncB64 = B64Enc(&szB64_2,rgOrgData2,ncData2);
	if( ncB64<1 )
	{
		MessageBox(NULL,TEXT("Base64-Kodierung fehlgeschlagen!"),TEXT("Test2"),MB_ICONERROR);
		return;
	}
	ncData = B64Dec(&rgDecData2,szB64_2);
	if( ncData<1 )
	{
		MessageBox(NULL,TEXT("Base64-Dekodierung fehlgeschlagen!"),TEXT("Test2"),MB_ICONERROR);
		free(szB64_2);
		return;
	}

	
	_snprintf(	szOut,sizeof(szOut),"Orginal-String:\t%s\nB64-String:\t%s\nDecodierter String:\t%s",
				rgOrgData2,szB64_2,rgDecData2);
	MessageBoxA(NULL,szOut,"Test2",MB_ICONINFORMATION);
	free(szB64_2);
	free(rgDecData2);
#endif
}

