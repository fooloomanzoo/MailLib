/***********************************************************************************/
/* Autor:  Benjamin Bruns                  Telefon: 3082                           */
/* E-Mail: b.bruns@fz-juelich.de           Zeitraum: 2006                          */
/*                                                                                 */
/***********************************************************************************/
/* MapiMail-Modul: Versenden einer Email (ggf. mit Anhängen und Kopienempfängnern) */
/*                 über die (Simple-)Mapi                                          */
/*                                                                                 */
/***********************************************************************************/
/* Programmiersprache: C        *                                                  */
/*                              * Rechner:  PC INTEL PENTIUM 4 3 GHz               */
/*                              *                                                  */
/* Compiler: Visual C++         *                                                  */
/*      Version 2005            * Betriebssystem: Windows XP SP2                   */
/*                                                                                 */
/***********************************************************************************/
/* Abhängigkeiten:    MAPI32.DLL                                                   */
/*                    Util-Modul                                                   */
/*                                                                                 */
/***********************************************************************************/
#include <windows.h>
#include <TCHAR.h>
#include <stdio.h>
#include <mapi.h>
#include "../MailLib.h"
#include "MapiMail.h"
#include "../Utils/Util.h"

#define MAX_DEBUG_SIZE 199

/************************************************************************************/
/* Name der Funktion: mapi_send_mail                                                */
/*                                                                                  */
/* Aufgabe: Versendet über die (Simple-)Mapi eine Email                             */
/*          Information über den Sender und den Smtp-Server werden von (Mapi-)      */
/*          Hauptidentität übernommen                                               */
/*                                                                                  */
/* Eingabeparameter: m (beinhaltet alle notw. Informationen zum E-Mailversand)      */
/*                   szDebug (Stringpuffer für evtl. Fehlerbeschreibung)            */
/*                           (kann NULL sein)                                       */
/*                                                                                  */
/* Rückgabewerte:    Erfolg des Emailversands                                       */
/*                                                                                  */
/* Verwendete Funktionen: MapiLogon, MapiLogoff, MapiSendMail (alle aus MAPI32.DLL) */
/*                                                                                  */
/************************************************************************************/
BOOL mapi_send_mail(MailType* m, TCHAR* szDebug)
{
	HINSTANCE hMapi = NULL;
	LPMAPILOGON pfMapiLogon;
	LPMAPILOGOFF pfMapiLogoff;
	LPMAPISENDMAIL pfMapiSendMail;
	LHANDLE hSession = 0;
	MapiRecipDesc *pstRecDesc = NULL;
	MapiFileDesc *pstFilesDesc = NULL;
	MapiMessage MapiMsg;
	BOOL bRes;

	int iRecips, i;
	size_t l;
	TCHAR *ptr;

	if (m == NULL || m->RecipVec == NULL || m->iRecip < 1 || m->szSubject == NULL || m->szSenderAddress == NULL)
		return FALSE;

	if (m->BccRecipVec == NULL || m->iBccRecip < 0)
		m->iBccRecip = 0;
	if (m->CcRecipVec == NULL || m->iCcRecip < 0)
		m->iCcRecip = 0;
	if (m->AttachVec == NULL || m->iAttach < 0)
		m->iAttach = 0;

	if (szDebug)
		lstrcpy(szDebug, TEXT(""));

	memset(&MapiMsg, 0, sizeof(MapiMessage));

	hMapi = LoadLibrary(TEXT("MAPI32.DLL"));
	if (hMapi == NULL)
	{
		if (szDebug)
			_sntprintf(szDebug, MAX_DEBUG_SIZE - 1, TEXT("Mapi32.dll konnte nicht geladen werden!"));
		return FALSE;
	}

	pfMapiLogon = (LPMAPILOGON)GetProcAddress(hMapi, "MAPILogon");
	pfMapiLogoff = (LPMAPILOGOFF)GetProcAddress(hMapi, "MAPILogoff");
	pfMapiSendMail = (LPMAPISENDMAIL)GetProcAddress(hMapi, "MAPISendMail");

	if (pfMapiLogon == NULL || pfMapiLogoff == NULL || pfMapiSendMail == NULL)
	{
		FreeLibrary(hMapi);
		if (szDebug)
			_sntprintf(szDebug, MAX_DEBUG_SIZE - 1, TEXT("Mapi-Funktionsadressen konnte nicht aufgelöst werden!"));
		return FALSE;
	}

	if (pfMapiLogon(0, NULL, NULL, MAPI_LOGON_UI, 0, &hSession) != SUCCESS_SUCCESS)
	{
		FreeLibrary(hMapi);
		if (szDebug)
			_sntprintf(szDebug, MAX_DEBUG_SIZE - 1, TEXT("MAPILogon fehlgeschlagen!"));
		return FALSE;
	}

	iRecips = m->iRecip + m->iCcRecip + m->iBccRecip;
	pstRecDesc = (MapiRecipDesc*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (iRecips + 1) * sizeof(MapiRecipDesc));

	if ((bRes = pstRecDesc != NULL) == TRUE)
	{
		memset(pstRecDesc + iRecips, 0, sizeof(MapiRecipDesc));
		pstRecDesc[iRecips].ulRecipClass = MAPI_ORIG;
		l = lstrlen(m->szSenderAddress) + 5 + 1;
		pstRecDesc[iRecips].lpszAddress = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (l) * sizeof(char));
		if (pstRecDesc[iRecips].lpszAddress == NULL)
			bRes = FALSE;
		else
		{
			strcpy(pstRecDesc[iRecips].lpszAddress, "SMTP:");
			Tchar2Ansi(m->szSenderAddress, pstRecDesc[iRecips].lpszAddress + 5, l - 5);

			l = lstrlen(m->szSenderName ? m->szSenderName : m->szSenderAddress) + 1;
			pstRecDesc[iRecips].lpszName = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ((l) * sizeof(char)));
			if (pstRecDesc[iRecips].lpszName == NULL)
				bRes = FALSE;
			else
				Tchar2Ansi(m->szSenderName ? m->szSenderName : m->szSenderAddress, pstRecDesc[iRecips].lpszName, l);
		}
	}

	for (i = 0; bRes && i < m->iRecip; ++i)
	{
		memset(pstRecDesc + i, 0, sizeof(MapiRecipDesc));
		pstRecDesc[i].ulRecipClass = MAPI_TO;
		l = lstrlen(m->RecipVec[i]) + 1;
		pstRecDesc[i].lpszName = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (l) * sizeof(char));
		if (pstRecDesc[i].lpszName == NULL)
			break;
		Tchar2Ansi(m->RecipVec[i], pstRecDesc[i].lpszName, l);
		l = lstrlen(m->RecipVec[i]) + 5 + 1;
		pstRecDesc[i].lpszAddress = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (l) * sizeof(char));
		if (pstRecDesc[i].lpszAddress == NULL)
			break;
		strcpy(pstRecDesc[i].lpszAddress, "SMTP:");
		Tchar2Ansi(m->RecipVec[i], pstRecDesc[i].lpszAddress + 5, l - 5);
	}
	if (i != m->iRecip)
	{
		if (pstRecDesc[i].lpszName)
			HeapFree(GetProcessHeap(), 0, pstRecDesc[i].lpszName);
		bRes = FALSE;
		m->iRecip = i;
	}

	for (i = 0; bRes && i < m->iCcRecip; ++i)
	{
		memset(pstRecDesc + i + m->iRecip, 0, sizeof(MapiRecipDesc));
		pstRecDesc[i + m->iRecip].ulRecipClass = MAPI_CC;
		l = lstrlen(m->CcRecipVec[i]) + 1;
		pstRecDesc[i + m->iRecip].lpszName = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (l) * sizeof(char));
		if (pstRecDesc[i + m->iRecip].lpszName == NULL)
			break;
		Tchar2Ansi(m->CcRecipVec[i], pstRecDesc[i + m->iRecip].lpszName, l);

		l = lstrlen(m->CcRecipVec[i]) + 5 + 1;
		pstRecDesc[i + m->iRecip].lpszAddress = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (l) * sizeof(char));
		if (pstRecDesc[i + m->iRecip].lpszAddress == NULL)
			break;
		strcpy(pstRecDesc[i + m->iRecip].lpszAddress, "SMTP:");
		Tchar2Ansi(m->CcRecipVec[i], pstRecDesc[i + m->iRecip].lpszAddress + 5, l - 5);
	}
	if (i != m->iCcRecip)
	{
		if (pstRecDesc[i + m->iRecip].lpszName)
			HeapFree(GetProcessHeap(), 0, pstRecDesc[i + m->iRecip].lpszName);
		bRes = FALSE;
		m->iCcRecip = i;
	}

	for (i = 0; bRes && i < m->iBccRecip; ++i)
	{
		memset(pstRecDesc + i + m->iRecip, 0, sizeof(MapiRecipDesc));
		pstRecDesc[i + m->iRecip + m->iBccRecip].ulRecipClass = MAPI_BCC;
		l = lstrlen(m->BccRecipVec[i]) + 1;
		pstRecDesc[i + m->iRecip + m->iBccRecip].lpszName = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (l) * sizeof(char));
		if (pstRecDesc[i + m->iRecip + m->iBccRecip].lpszName == NULL)
			break;
		Tchar2Ansi(m->BccRecipVec[i], pstRecDesc[i + m->iRecip + m->iBccRecip].lpszName, l);

		l = lstrlen(m->BccRecipVec[i]) + 5 + 1;
		pstRecDesc[i + m->iRecip + m->iBccRecip].lpszAddress = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (l) * sizeof(char));
		if (pstRecDesc[i + m->iRecip + m->iBccRecip].lpszAddress == NULL)
			break;
		strcpy(pstRecDesc[i + m->iRecip + m->iBccRecip].lpszAddress, "SMTP:");
		Tchar2Ansi(m->BccRecipVec[i], pstRecDesc[i + m->iRecip + m->iBccRecip].lpszAddress + 5, l - 5);
	}
	if (i != m->iBccRecip)
	{
		if (pstRecDesc[i + m->iRecip + m->iBccRecip].lpszName)
			HeapFree(GetProcessHeap(), 0, pstRecDesc[i + m->iRecip + m->iBccRecip].lpszName);
		bRes = FALSE;
		m->iBccRecip = i;
	}

	if (bRes)
		pstFilesDesc = (MapiFileDesc*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (m->iAttach * sizeof(MapiFileDesc)));

	for (i = 0; bRes && i < m->iAttach; ++i)
	{
		memset(pstFilesDesc + i, 0, sizeof(MapiFileDesc));
		pstFilesDesc[i].nPosition = (ULONG)-1;
		l = lstrlen(m->AttachVec[i]) + 1;
		pstFilesDesc[i].lpszPathName = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (l) * sizeof(char));
		if (pstFilesDesc[i].lpszPathName == NULL)
			break;
		Tchar2Ansi(m->AttachVec[i], pstFilesDesc[i].lpszPathName, l);
		ptr = _tcsrchr(m->AttachVec[i], TEXT('\\'));
		l = lstrlen(ptr ? ptr + 1 : TEXT("")) + 1;
		pstFilesDesc[i].lpszFileName = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (l) * sizeof(char));
		if (pstFilesDesc[i].lpszFileName == NULL)
			break;
		Tchar2Ansi(ptr ? ptr + 1 : TEXT(""), pstFilesDesc[i].lpszFileName, l);
	}
	if (i != m->iAttach)
	{
		if (pstFilesDesc[i].lpszPathName)
			HeapFree(GetProcessHeap(), 0, pstFilesDesc[i].lpszPathName);
		bRes = FALSE;
		m->iAttach = i;
	}

	if (!bRes && szDebug)
		_sntprintf(szDebug, MAX_DEBUG_SIZE - 1, TEXT("Speicherallokationsfehler!"));

	if (bRes)
	{  /*if( bRes )*/
		ULONG uRes;

		l = lstrlen(m->szSubject) + 1;
		MapiMsg.lpszSubject = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (l) * sizeof(char));
		if ((bRes = MapiMsg.lpszSubject != NULL) == TRUE)
		{
			Tchar2Ansi(m->szSubject, MapiMsg.lpszSubject, l);
			l = lstrlen(m->szMail ? m->szMail : TEXT("")) + 1;
			MapiMsg.lpszNoteText = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (l) * sizeof(char));
		}

		if ((bRes = bRes&&MapiMsg.lpszNoteText != NULL) == TRUE)
		{ /*if( bRes )*/
			Tchar2Ansi(m->szMail ? m->szMail : TEXT(""), MapiMsg.lpszNoteText, l);
			MapiMsg.nRecipCount = (ULONG)iRecips;
			MapiMsg.lpRecips = pstRecDesc;
			MapiMsg.nFileCount = (ULONG)m->iAttach;
			MapiMsg.lpFiles = pstFilesDesc;
			MapiMsg.lpOriginator = pstRecDesc + iRecips;
			uRes = pfMapiSendMail(hSession, 0, &MapiMsg, MAPI_LOGON_UI, 0);
			bRes = uRes == SUCCESS_SUCCESS;

			if (!bRes && szDebug)
			{ /*if( !bRes && szDebug )*/
				switch (uRes)
				{ /*switch( uRes )*/
				case MAPI_E_AMBIGUOUS_RECIPIENT:
					_sntprintf(szDebug, MAX_DEBUG_SIZE - 1, TEXT("MAPISendMail: A recipient matched more than one of the recipient descriptor structures and MAPI_DIALOG was not set. No message was sent."));
					break;
				case MAPI_E_ATTACHMENT_NOT_FOUND:
					_sntprintf(szDebug, MAX_DEBUG_SIZE - 1, TEXT("MAPISendMail: The specified attachment was not found. No message was sent."));
					break;
				case MAPI_E_ATTACHMENT_OPEN_FAILURE:
					_sntprintf(szDebug, MAX_DEBUG_SIZE - 1, TEXT("MAPISendMail: The specified attachment could not be opened. No message was sent."));
					break;
				case MAPI_E_BAD_RECIPTYPE:
					_sntprintf(szDebug, MAX_DEBUG_SIZE - 1, TEXT("MAPISendMail: The type of a recipient was not MAPI_TO, MAPI_CC, or MAPI_BCC. No message was sent."));
					break;
				case MAPI_E_FAILURE:
					_sntprintf(szDebug, MAX_DEBUG_SIZE - 1, TEXT("MAPISendMail: One or more unspecified errors occurred. No message was sent."));
					break;
				case MAPI_E_INSUFFICIENT_MEMORY:
					_sntprintf(szDebug, MAX_DEBUG_SIZE - 1, TEXT("MAPISendMail: There was insufficient memory to proceed. No message was sent."));
					break;
				case MAPI_E_INVALID_RECIPS:
					_sntprintf(szDebug, MAX_DEBUG_SIZE - 1, TEXT("MAPISendMail: One or more recipients were invalid or did not resolve to any address."));
					break;
				case MAPI_E_LOGIN_FAILURE:
					_sntprintf(szDebug, MAX_DEBUG_SIZE - 1, TEXT("MAPISendMail: There was no default logon, and the user failed to log on successfully when the logon dialog box was displayed. No message was sent."));
					break;
				case MAPI_E_TEXT_TOO_LARGE:
					_sntprintf(szDebug, MAX_DEBUG_SIZE - 1, TEXT("MAPISendMail: The text in the message was too large. No message was sent."));
					break;
				case MAPI_E_TOO_MANY_FILES:
					_sntprintf(szDebug, MAX_DEBUG_SIZE - 1, TEXT("MAPISendMail: There were too many file attachments. No message was sent."));
					break;
				case MAPI_E_TOO_MANY_RECIPIENTS:
					_sntprintf(szDebug, MAX_DEBUG_SIZE - 1, TEXT("MAPISendMail: There were too many recipients. No message was sent."));
					break;
				case MAPI_E_UNKNOWN_RECIPIENT:
					_sntprintf(szDebug, MAX_DEBUG_SIZE - 1, TEXT("MAPISendMail: A recipient did not appear in the address list. No message was sent."));
					break;
				case MAPI_E_USER_ABORT:
					_sntprintf(szDebug, MAX_DEBUG_SIZE - 1, TEXT("MAPISendMail: The user canceled one of the dialog boxes. No message was sent."));
					break;
				case SUCCESS_SUCCESS:
					_sntprintf(szDebug, MAX_DEBUG_SIZE - 1, TEXT("MAPISendMail: The call succeeded and the message was sent."));
					break;
				default:
					_sntprintf(szDebug, MAX_DEBUG_SIZE - 1, TEXT("MAPISendMail: Unkown return code"));
				} /*switch( uRes )*/
			} /*if( !bRes && szDebug )*/
		} /*if( bRes )*/
	} /*if( bRes )*/

	pfMapiLogoff(hSession, 0, 0, 0);
	FreeLibrary(hMapi);

	for (i = 0; i < iRecips; ++i)
	{
		if (pstRecDesc[i].lpszName)
			HeapFree(GetProcessHeap(), 0, pstRecDesc[i].lpszName);
		if (pstRecDesc[i].lpszAddress)
			HeapFree(GetProcessHeap(), 0, pstRecDesc[i].lpszAddress);
	}
	if (pstRecDesc[i].lpszName)
		HeapFree(GetProcessHeap(), 0, pstRecDesc[i].lpszName);
	if (pstRecDesc[i].lpszAddress)
		HeapFree(GetProcessHeap(), 0, pstRecDesc[i].lpszAddress);
	if (pstRecDesc)
		HeapFree(GetProcessHeap(), 0, pstRecDesc);
	for (i = 0; i < m->iAttach; ++i)
	{
		if (pstFilesDesc[i].lpszPathName)
			HeapFree(GetProcessHeap(), 0, pstFilesDesc[i].lpszPathName);
		if (pstFilesDesc[i].lpszFileName)
			HeapFree(GetProcessHeap(), 0, pstFilesDesc[i].lpszFileName);
	}
	if (pstFilesDesc)
		HeapFree(GetProcessHeap(), 0, pstFilesDesc);

	if (MapiMsg.lpszNoteText)
		HeapFree(GetProcessHeap(), 0, MapiMsg.lpszNoteText);

	if (MapiMsg.lpszSubject)
		HeapFree(GetProcessHeap(), 0, MapiMsg.lpszSubject);

	return bRes;
}
