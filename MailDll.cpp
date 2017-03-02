/***********************************************************************************/
/* Autor:  Benjamin Bruns                  Telefon: 3082                           */
/* E-Mail: b.bruns@fz-juelich.de           Zeitraum: 2006                          */
/*                                                                                 */
/* ergänzt von Werner Hürttlen    Tel. 4357 02.11.2007                             */
/* ergänzt von Johannes Brautzsch Tel. 2906 01.03.2017                             */
/*                                                                                 */
/*                                                                                 */
/***********************************************************************************/
/* MailDll-Modul: Stellt über eine Dll Methoden zum Emailversand bereit            */
/*                                                                                 */
/***********************************************************************************/
/*                                                                                 */
/***********************************************************************************/
/* Abhängigkeiten:    MAPI, Pop3- und Smtp-Modul                                   */
/*                                                                                 */
/***********************************************************************************/

#define MAIL_DLL_EXPORTS

#include "Smtp/CSmtp.h"

extern "C" {
	#include "Pop3/Pop3.h"
	#include "Mapi/MapiMail.h"
}

#include "MailDll.h"

#pragma comment(lib, "ws2_32.lib")

#define MAILLIB_DLL_NAME TEXT("MAILLIB.DLL")
#define RETURNSIZE 255
#define STRINGSIZE 255
#define EMAILSIZE 65535

#ifndef DLL_MAIN
#define DLL_MAIN
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}
#endif

/********************************************************************************************/
// Funktion: bool SmtpSendMail( MailType* m )			 										
//
// Zweck:
//   Sendet Email an Smtp-Server
//
// Parameter:
//	 MailType* m 
//
//   Return:
//	   true wenn OK oder false wenn fehlgeschlagen
//
//  Abhängigkeiten:
//   CSmtp.h,
//   MailLib.h
/********************************************************************************************/

MAIL_DLL_EXPORT bool SmtpSendMail(MailType* m)
{
	if ( !m->szSmtpServer )
	{
		return false;
	}

	bool NoErrorHasOccurred = true;
	try
	{
		CSmtp mail;
		int i, iRecip, iCcRecip, iBccRecip, iAttach;

		// Verbindungseinstellungen

		if ( !m->szSmtpServerPort ) 
		{ /* Benutze Standard-Ports, wenn Port nicht gesetzt */
			switch ( m->szSmtpSecurityType )
			{
			case USE_TLS: /*TLS*/
				m->szSmtpServerPort = 587;
				break;
			case USE_SSL: /*SSL*/
				m->szSmtpServerPort = 465;
				break;
			default: /*NO_SECURITY & DO_NOT_SET*/
				m->szSmtpServerPort = 25;
				break;
			}
		}

		if ( !m->szSmtpSecurityType ) 
		{ /* Benutze keine Verschlüsselung, wenn nicht gesetzt */
			m->szSmtpSecurityType = NO_SECURITY;
		}

		// Login
		if (m->szSmtpUser && m->szSmtpPass)
		{
			mail.SetLogin(m->szSmtpUser);
			mail.SetPassword(m->szSmtpPass);
		} 
		else if (m->szPop3User && m->szPop3Pass)
		{ /* für ältere Implementationen (da szSmtpUser und szSmtpPass erst später implementiert wurden) */
			mail.SetLogin(m->szPop3User);
			mail.SetPassword(m->szPop3Pass);
		}

		// Servereinstellungen
		mail.SetSMTPServer( m->szSmtpServer, m->szSmtpServerPort, true );
		mail.SetSecurityType( m->szSmtpSecurityType );

		// Sendepriorität
		if ( !m->szSmtpPriority )
		{
			m->szSmtpPriority = XPRIORITY_NORMAL;
		}
		mail.SetXPriority( m->szSmtpPriority );

		// Absenderinformationen
		mail.SetSenderName( m->szSenderName );
		mail.SetSenderMail( m->szSenderAddress );
		mail.SetReplyTo( m->szReplyAddress );
		mail.SetXMailer( m->szMailerName );

		// Empfänger
		if (m->RecipVec && sizeof(m->RecipVec) && sizeof(m->RecipVec[0]))
		{
			iRecip = sizeof(m->RecipVec) / sizeof(m->RecipVec[0]);
			for (i = 0; i < iRecip; ++i)
			{ /* Füge Empfänger hinzu */
				mail.AddRecipient(m->RecipVec[i]);
			}
		}

		// CC-Empfänger
		if (m->CcRecipVec && sizeof(m->CcRecipVec) && sizeof(m->CcRecipVec[0]))
		{
			iCcRecip = sizeof(m->CcRecipVec) / sizeof(m->CcRecipVec[0]);
			for (i = 0; i < iCcRecip; ++i)
			{ /* Füge CC-Empfänger hinzu */
				mail.AddCCRecipient( m->CcRecipVec[i] );
			}
		}
		if (m->BccRecipVec && sizeof(m->BccRecipVec) && sizeof(m->BccRecipVec[0]))
		{
			// Anzahl der BCC-Empfänger
			iBccRecip = sizeof(m->BccRecipVec) / sizeof(m->BccRecipVec[0]);
			for (i = 0; i < iBccRecip; ++i)
			{ /* Füge Bcc-Empfänger hinzu */
				mail.AddBCCRecipient( m->BccRecipVec[i] );
			}
		}

		// Main-Inhalt und -Betreff
		mail.SetSubject( m->szSubject );
		mail.AddMsgLine( m->szMail );

		// Anhänge
		if (m->AttachVec && sizeof(m->AttachVec) && sizeof(m->AttachVec[0]))
		{
			iAttach = sizeof(m->AttachVec) / sizeof(m->AttachVec[0]);
			for (i = 0; i < iAttach; ++i)
			{ /* Füge Anhänge hinzu */
				mail.AddAttachment(m->AttachVec[i]);
			}
		}

		// Senden der Email
		mail.Send();
	}
	catch ( ECSmtp e )
	{
		std::cout << "Error: " << e.GetErrorText().c_str() << ".\n";
		NoErrorHasOccurred = false;
	}
	catch (const std::exception& e) {
		std::cout << "Error: " << e.what();
		NoErrorHasOccurred = false;
	}
	catch (...)
	{
		std::cout << "Undefined Error\n";
		NoErrorHasOccurred = false;
	}

	if (NoErrorHasOccurred == true)
	{
		std::cout << "Mail was send successfully.\n";
	}

	return NoErrorHasOccurred;
}



/********************************************************************************************/
// Funktion: int MailLibCopyFile(char* szSrcFile, char* szDestFile)						 										
//
// Zweck:
// Kopiert Quellfile mit Pfad nach Ziel
//
//	Parameter:
//	char* szSrcFile, 
//	char* szDestFile
//
//	Return:
//	int 1 wenn OK oder 0 wenn fehlgeschlagen
/********************************************************************************************/

MAIL_DLL_EXPORT int MailLibCopyFile(char* szSrcFile, char* szDestFile)
{
	return CopyFile(szSrcFile, szDestFile, FALSE);

}

/********************************************************************************************/
// Funktion: int MailLibInit(char* szSmtpServer, char* szSenderAddress)						 										
//
// Zweck:
// Initialisiert die Mailstruktur mit Standardparametern
//
//	Parameter:
//	char* szSmtpServer, 
//	char* szSenderAddress
//	char* szSenderName, 
//	char* szMailerName
//
//	Return:
//	int 1 wenn OK oder 0 wenn fehlgeschlagen
/********************************************************************************************/

//Beginn der Initialierung der Mail-Parameter-Struktur
//globale Struktur anlegen
MailType grMail;
char szSmtpMailServer[STRINGSIZE];
char szAbsenderEmailAddress[STRINGSIZE];
char szAbsenderName[STRINGSIZE];
char szMailprogrammName[STRINGSIZE];


MAIL_DLL_EXPORT int MailLibInit(char* szSmtpServer, char* szSenderAddress, char* szSenderName, char* szMailerName)
{
	// Smtp-User Parameter setzen
	grMail.szSmtpUser = NULL; //Authentifizierung
	grMail.szSmtpPass = NULL;  //Authentifizierung
	grMail.szReplyAddress = NULL;

	// Smtp-Server Parameter setzen
	strcpy(szSmtpMailServer, szSmtpServer);
	grMail.szSmtpServer = szSmtpMailServer;
	grMail.szSmtpServerPort = 25;

	// Mail Parameter setzen
	grMail.szSmtpSecurityType = NO_SECURITY;
	grMail.szSmtpPriority = XPRIORITY_NORMAL;

	// Anhang
	grMail.iAttach = 0;
	grMail.AttachVec = NULL;

	// Absenderadresse
	strcpy(szAbsenderEmailAddress, szSenderAddress);
	grMail.szSenderAddress = szAbsenderEmailAddress;

	strcpy(szAbsenderName, szSenderName);
	grMail.szSenderName = szAbsenderName;

	strcpy(szMailprogrammName, szMailerName);
	grMail.szMailerName = szMailprogrammName;

	return TRUE;
}


/********************************************************************************************/
// Funktion: int MailLibSendMail(char* szEmpfaengerAddress, char* szSubject, char* szMail)						 										
//
// Zweck:
// Initialisiert die Mailstruktur mit Standardparametern
//
//	Parameter:
//	char* szEmpfaengerAddress, 
//	char* szSubject, 
//	char* szMail
//
//	Return:
//	int 1 wenn OK oder 0 wenn fehlgeschlagen
/********************************************************************************************/
char szEmpfEmailAddress[STRINGSIZE];
char** lpRecip;
char szEmpfSubject[STRINGSIZE];
char szEmailInhalt[EMAILSIZE];


MAIL_DLL_EXPORT int MailLibSendMail(char* szRecip, char* szSubject, char* szMail)
{
	// Empfänger
	grMail.iRecip = RecipSplit(&lpRecip, szRecip);
	grMail.RecipVec = lpRecip;

	// Betreff
	strcpy(szEmpfSubject, szSubject);
	grMail.szSubject = szEmpfSubject;

	// Inhalt
	strcpy(szEmailInhalt, szMail);
	grMail.szMail = szEmailInhalt;

	// Senden der Email
	return SmtpSendMail(&grMail);
}


BOOL MAIL_DLL_EXPORT MapiSendMail(MailType* m)
{
	//return (BOOL) mapi_send_mail(m, NULL);
	return TRUE;
}


/********************************************************************************************/
// Funktion: int MailLibAddAttachement(char* szTextAttachment)						 										
//
// Zweck:
// Initialisiert die Mailstruktur mit Standardparametern
//
//	Parameter:
//	char* szTextAttachment
//
//	Return:
//	int 1 wenn OK oder 0 wenn fehlgeschlagen
/********************************************************************************************/
char szAttachment[STRINGSIZE];
char* lpAttachment[1];

MAIL_DLL_EXPORT int MailLibAddAttachement(char* szTextAttachment)
{
	// Pfad zur Text-Datei, die Anhang werden soll	
	strcpy(szAttachment, szTextAttachment);
	lpAttachment[0] = szAttachment;

	//Pfad der zu versendenden Datei
	grMail.AttachVec = lpAttachment;

	// Anzahl der zu versendenden Datei ist 1, wenn das erste Zeichen des Pfads nicht 0 ist.
	if (*szTextAttachment == 0)
		grMail.iAttach = 0;
	else
		grMail.iAttach = 1;

	return TRUE;
}


/********************************************************************************************/
// Funktion: int MailLibAuthentifizierung(char* szUsername, char* szUserPassword)						 										
//
// Zweck:
//   Setzt Authentifizierungsparameter (gleich für Pop3 und Smtp)
//
//	Parameter:
//	char* szUsername, 
//	char* szUserPassword
//
//	Return:
//	int 1 wenn OK oder 0 wenn fehlgeschlagen
/********************************************************************************************/
char szUserName[STRINGSIZE];
char szUserPasswort[STRINGSIZE];

MAIL_DLL_EXPORT int MailLibAuthentifizierung(char* szUsername, char* szUserPassword)
{
	// Smtp/Pop3Server Authentifizierungsparameter setzen
	strcpy(szUserName, szUsername);
	grMail.szPop3User = szUserName;
	grMail.szSmtpUser = szUserName;

	strcpy(szUserPasswort, szUserPassword);
	grMail.szPop3Pass = szUserPasswort;
	grMail.szSmtpPass = szUserPasswort;

	return TRUE;
}


/********************************************************************************************/
// Funktion: int Pop3ReceiveMail(MailType* pMailData, BOOL bDeleteMail, const TCHAR* szAppendPath)						 										
//
// Zweck:
// Pop3 Mail Empfang
//
// Kommentar: Ungetestet & nicht über SSL/TLS
/********************************************************************************************/

INT MAIL_DLL_EXPORT Pop3ReceiveMail(MailType* pMailData, BOOL bDeleteMail, const TCHAR* szAppendPath)
{
	//return (INT) Pop3_Receive_Mail(pMailData, bDeleteMail, szAppendPath, NULL);
	return 0;
}


/********************************************************************************************/
// Funktion: bool Pop3FreeData(MailType* pMailData)		 										
//
// Zweck:
// Freimachung des Speichers
//
// Kommentar: Ungetestet
/********************************************************************************************/

BOOL MAIL_DLL_EXPORT Pop3FreeData(MailType* pMailData)
{
	//return (BOOL) Pop3_Free_Data(pMailData);
	return TRUE;
}

/********************************************************************************************/
// Funktion: int RecipSplit(char*** rgRecips, char* adresses)						 										
//
// Zweck:
// Parst den Empfängerstring und speichert die einzelnen email-Adressen auf einem char-Feld
// gibt die Anzahl der Empfänger zurück
//
//	Parameter:
//	char*** rgRecips
//		Pointer auf das Feld, in dem die email-Adressen gespeichert werden sollen
//  char* adresses
//		Adressen-String
//
//	Return:
//	int Anzahl der Empfänger oder 0 wenn fehlgeschlagen
/********************************************************************************************/

MAIL_DLL_EXPORT int RecipSplit(char*** rgRecips, char* adresses)
{
	char * help;
	char str[STRINGSIZE] = "";
	int iRecip = 0;

	strcpy(str, adresses);

	help = strtok(str, ";,");
	while (help != NULL)
	{
		iRecip++;
		*rgRecips = (char**)realloc((*rgRecips), iRecip * sizeof(char*));
		(*rgRecips)[iRecip - 1] = (char*)malloc(EMAILSIZE * sizeof(char));
		strcpy((*rgRecips)[iRecip - 1], help);
		help = strtok(NULL, ";,");
	}
	return iRecip;
}


/********************************************************************************************/
// Funktion: int SecSeit1970( int nDay,int nMonth,int nYear,int nHour,int nMinute)						 										
//
// Zweck:
// Funktion die aus Datum/Uhrzeit die vergangenen Sekunden seit dem 1.1.1970 zurückliefert
//
//	Parameter:
//
//	Return:
//	int 1 wenn OK oder 0 wenn fehlgeschlagen
/********************************************************************************************/
MAIL_DLL_EXPORT int SecSeit1970(int nDay, int nMonth, int nYear, int nHour, int nMinute)
{
	int nSekunden;
	struct tm TM;

	TM.tm_sec = 0;      /* Sekunden - [0,61] */
	TM.tm_min = nMinute;      /* Minuten - [0,59] */
	TM.tm_hour = nHour;     /* Stunden - [0,23] */
	TM.tm_mday = nDay;     /* Tag des Monats - [1,31] */
	TM.tm_mon = nMonth - 1;      /* Monat im Jahr - [0,11] */
	TM.tm_year = nYear - 1900;     /* Jahr seit 1900 */
								   //TM.tm_wday;     /* Tage seit Sonntag (Wochentag) - [0,6] */
								   //TM.tm_yday;     /* Tage seit Neujahr (1.1.) - [0,365] */
	TM.tm_isdst = -1;    /* Sommerzeit-Flag */

	nSekunden = (int) mktime(&TM);

	return nSekunden;
}


/********************************************************************************************/
// Funktion: int MailLibVerInfo(char *szInfo)						 										
//
// Zweck:
// liefert eine Versionsinfo aus der entsprechenden Ressource in den übergebenen String zurück
//
//	Parameter:
//	char* szInfo	
//	vom Aufrufer ausreichend gross (mind. 32 Zeichen!!!) bereitzustellen, hier wird die
//	Versionsinfo reinkopiert
//
//	Return:
//	negativ oder 0=Fehler  1=TRUE=ok
/********************************************************************************************/


// DWORD   dwSignature;            /* e.g. 0xfeef04bd */
// DWORD   dwStrucVersion;         /* e.g. 0x00000042 = "0.42" */
// DWORD   dwFileVersionMS;        /* e.g. 0x00030075 = "3.75" */
// DWORD   dwFileVersionLS;        /* e.g. 0x00000031 = "0.31" */
// DWORD   dwProductVersionMS;     /* e.g. 0x00030010 = "3.10" */
// DWORD   dwProductVersionLS;     /* e.g. 0x00000031 = "0.31" */
// DWORD   dwFileFlagsMask;        /* = 0x3F for version "0.42" */
// DWORD   dwFileFlags;            /* e.g. VFF_DEBUG | VFF_PRERELEASE */
// DWORD   dwFileOS;               /* e.g. VOS_DOS_WINDOWS16 */
// DWORD   dwFileType;             /* e.g. VFT_DRIVER */
// DWORD   dwFileSubtype;          /* e.g. VFT2_DRV_KEYBOARD */
// DWORD   dwFileDateMS;           /* e.g. 0 */
// DWORD   dwFileDateLS;           /* e.g. 0 */


MAIL_DLL_EXPORT int MailLibVerInfo(char *szInfo)
{
	TCHAR szDllPath[MAX_PATH];
	//	TCHAR szVerInfo[MAX_PATH] = "CABURDLL: ";
	TCHAR szVerInfo[MAX_PATH];
	TCHAR szDescription[MAX_PATH];
	TCHAR szVersion[MAX_PATH];

	DWORD dwMajorVersion = -1;
	DWORD dwMinorVersion = -1;

	BOOL bDescriptionRead = FALSE;
	BOOL bVersionRead = FALSE;

	HMODULE hDll;

	if (szInfo == NULL)
		return -1;


	hDll = GetModuleHandle(MAILLIB_DLL_NAME);
	if (hDll == NULL)
		return -2;

	if (GetModuleFileName(hDll, szDllPath, MAX_PATH) != 0)
	{
		DWORD dwInfoSize, dwDummy;
		char* pcInfoBuffer = NULL;

		if ((dwInfoSize = GetFileVersionInfoSize(szDllPath, &dwDummy)) != 0)
		{
			VS_FIXEDFILEINFO* pFixedFile = NULL;
			UINT wInfoLen;

			pcInfoBuffer = (char*) malloc(dwInfoSize);

			if (GetFileVersionInfo(szDllPath, 0UL, dwInfoSize, pcInfoBuffer) != 0)
			{
				LPVOID pvInfo;

				//Description
				if (VerQueryValue(pcInfoBuffer, TEXT("\\VarFileInfo\\Translation"), &pvInfo, &wInfoLen) != 0)
				{
					TCHAR szFormat[80], *szData = NULL;
					WORD wCodePage = LOWORD(*(DWORD*)pvInfo);
					WORD wLangID = HIWORD(*(DWORD*)pvInfo);

					sprintf_s(szFormat, 80, TEXT("StringFileInfo\\%04X%04X\\FileDescription"), wCodePage, wLangID);
					if (VerQueryValue(pcInfoBuffer, szFormat, (void**)&szData, &wInfoLen) != 0)
					{
						strcpy_s(szDescription, MAX_PATH, szData);
						bDescriptionRead = TRUE;
					}
				}

				//Versionsinfo
				if (VerQueryValue(pcInfoBuffer, TEXT("\\"), (void**)&pFixedFile, &wInfoLen) != 0)
				{
					short nVersion1;
					short nVersion2;
					short nVersion3;
					short nVersion4;

					//codiert als HEX in HIWORD und LOWWORD
					dwMajorVersion = pFixedFile->dwFileVersionMS;
					dwMinorVersion = pFixedFile->dwFileVersionLS;

					//extrahieren in Variablen
					nVersion1 = HIWORD(dwMajorVersion);
					nVersion2 = LOWORD(dwMajorVersion);
					nVersion3 = HIWORD(dwMinorVersion);
					nVersion4 = LOWORD(dwMinorVersion);

					sprintf_s(szVersion, MAX_PATH, TEXT("%d.%d.%d.%d "), nVersion1, nVersion2, nVersion3, nVersion4);

					bVersionRead = TRUE;
				}
			}
		}

		if (pcInfoBuffer)
		{
			free(pcInfoBuffer);
			pcInfoBuffer = NULL;
		}
	}


	//Infos zusammenstellen
	//erst mal leer
	szVerInfo[0] = 0;

	if (bVersionRead)
		strcat_s(szVerInfo, MAX_PATH, szVersion);

	if (bDescriptionRead)
		strcat_s(szVerInfo, MAX_PATH, szDescription);


	strcpy_s(szInfo, RETURNSIZE, szVerInfo);


	if (bVersionRead == TRUE && bDescriptionRead == TRUE)
		return TRUE;
	else
		return FALSE;
}

/********************************************************************************************/
// Funktion: int MailLibDirInfo(char *szInfo)						 										
//
// Zweck:
// liefert das eigene DLL-Directory in den übergebenen String zurück
//
//	Parameter:
//	char* szInfo	
//	vom Aufrufer ausreichend gross (mind. 255 Zeichen!!!) bereitzustellen, hier wird der gesamte Pfad der DLL
//	reinkopiert
//
//	Return:
//	negativ oder 0=Fehler  1=TRUE=ok
/********************************************************************************************/
// Funktion die das eigene DLL-Directory zurückliefert

#define BUFSIZE MAX_PATH


MAIL_DLL_EXPORT int MailLibDirInfo(char *szInfo)
{
	HMODULE hDll;

	if (szInfo == NULL)
		return -1;

	/* Liefert nur den Ordner des Aufrufenden Prozesses zurück !
	TCHAR Buffer[BUFSIZE];
	DWORD dwRet;
	dwRet = GetCurrentDirectory(BUFSIZE, Buffer);
	if( dwRet == 0 )
	sprintf_s(Buffer, RETURNSIZE, TEXT("GetCurrentDirectory failed (%d)\n"), GetLastError());
	strcpy_s(szInfo, RETURNSIZE, Buffer);*/

	hDll = GetModuleHandle(MAILLIB_DLL_NAME);
	if (hDll == NULL)
	{
		strcpy_s(szInfo, RETURNSIZE, TEXT("NO HANDLE"));
		return -2;
	}
	else
	{
		if (GetModuleFileName(hDll, szInfo, MAX_PATH) == 0)
		{
			strcpy_s(szInfo, RETURNSIZE, TEXT("NO FileName"));
			return -3;
		}
	}

	return TRUE;
}

/********************************************************************************************/
// Funktion: int MailLibDirInfoSize(void)						 										
//
// Zweck:
// liefert die Größe des Strings der das eigene DLL-Directory hält zurück
//
//	Parameter:
//	void
//
//	Return:
//	int Größe in Zeichen oder 0 wenn fehlgeschlagen
/********************************************************************************************/
MAIL_DLL_EXPORT int MailLibDirInfoSize(void)
{
	TCHAR Buffer[RETURNSIZE];

	if (MailLibDirInfo(Buffer) == TRUE)
		return (int)strlen(Buffer);
	else
		return FALSE;
}
