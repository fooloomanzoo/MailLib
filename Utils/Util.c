/***********************************************************************************/
/* Autor:  Benjamin Bruns                  Telefon: 3082                           */
/* E-Mail: b.bruns@fz-juelich.de           Zeitraum: 2006                          */
/*                                                                                 */
/***********************************************************************************/
/* Util-Modul: Bietet allgemein nützliche Funktionen zur (Win32-)Programmierung an */
/*                                                                                 */
/***********************************************************************************/
/* Programmiersprache: C        *                                                  */
/*                              * Rechner:  PC INTEL PENTIUM 4 3 GHz               */
/*                              *                                                  */
/* Compiler: Visual C++         *                                                  */
/*      Version 2005            * Betriebssystem: Windows XP SP2                   */
/*                                                                                 */
/***********************************************************************************/
/* Abhängigkeiten:    keine                                                        */
/*                                                                                 */
/***********************************************************************************/

#ifdef _WIN32
#include <windows.h>
#include <TCHAR.h>

#else
#include <ctype.h>
#undef UNICODE
#define _istdigit isdigit
#define TEXT(__TEXT_ARG__) (__TEXT_ARG__)
#endif

#include <stdio.h>
#include "Util.h"



TCHAR* Ansi2Tchar(const char* conv, TCHAR* dest, size_t nChar)
{
	#ifdef UNICODE
		MultiByteToWideChar(CP_ACP,0,conv,-1,(wchar_t*)dest,(int)max(nChar-1,0));
	#else
		strncpy(dest,conv,max(nChar-1,0));
	#endif

	dest[nChar-1]=TEXT('\0');
	return dest;
}

char* Tchar2Ansi(const TCHAR* conv, char* dest, size_t nChar)
{
	#ifndef UNICODE
		strncpy(dest,conv,max(nChar-1,0));
	#else
		WideCharToMultiByte(CP_ACP,0,(wchar_t*)conv,-1,dest,(int)max(nChar-1,0),NULL,NULL);
	#endif
	
	dest[nChar-1]='\0';
	return dest;
}

BOOL CheckIp(const TCHAR* str)
{
	int i,j,k,numb;

	for(k=j=0; k<3; ++k)
	{
		for(numb=0,i=j; _istdigit(str[i]); ++i)
      	numb=numb*10+str[i]-TEXT('0');
		if( i-j>3 || j==i || str[i]!=TEXT('.') || numb>255 )
			return FALSE;
		j=i+1;
	}

	for(numb=0,i=j; _istdigit(str[i]); ++i)
   	numb=numb*10+str[i]-TEXT('0');
	if( i-j>3 || j==i || str[i]!=TEXT('\0') || numb>255 )
		return FALSE;

	return TRUE;
}
