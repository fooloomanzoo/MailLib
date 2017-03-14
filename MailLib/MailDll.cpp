/****************************************************************************/
/* Autor:  Benjamin Bruns                  Telefon: 3082                    */
/* E-Mail: b.bruns@fz-juelich.de           Zeitraum: 2006                   */
/*                                                                          */
/* ergaenzt von Werner Haerttlen    Tel. 4357 02.11.2007                    */
/* ergaenzt von Johannes Brautzsch Tel. 2906 03.03.2017                     */
/*                                                                          */
/****************************************************************************/
/* MailDll-Modul: Stellt ueber eine Dll Methoden zum Emailversand bereit    */
/*                                                                          */
/****************************************************************************/
/* Abhaengigkeiten:                                                         */
/*      Vorkompilierte Openssl-Bibliotheken von                             */
/*                        (https://slproweb.com/products/Win32OpenSSL.html) */
/*                    1. Installation von OpenSSL > v1.1.0                  */
/*                    2. Kopieren aus Installationsordner "include"- und    */
/*                       "lib"-Verzeichnis in "OpenSSL"-Ordner              */
/****************************************************************************/
#pragma once

#define MAIL_DLL_EXPORTS

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <windows.h>

#include "Smtp/CSmtp.h"
#include "MailDll.h"
#include "MailLib.h"

#define MAILLIB_DLL_NAME TEXT("MAILLIB.DLL")

// Globale Struktur anlegen
MailType grMail;

// DLL-MAIN
#ifndef DLL_MAIN
#define DLL_MAIN
#define strdup _strdup

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
		// Globale Struktur wird initialisiert
		if (grMail._initialized != TRUE)
			InitMailStruct(&grMail);
		break;
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		// Globale Struktur wird deallokiert
		if (grMail._initialized == TRUE)
			FreeMailStruct(&grMail);
		break;
	}
	return TRUE;
}
#endif

/****************************************************************************/
// Funktion: BOOL SmtpSendMail( MailType* m )
//
// Zweck:
//   Sendet Email an Smtp-Server
//
// Parameter:
//       MailType* m
//
//   Return:
//       TRUE wenn OK oder FALSE wenn fehlgeschlagen
//
//  Abhaengigkeiten:
//   CSmtp.h,
//   MailLib.h
/****************************************************************************/

MAIL_DLL_EXPORT BOOL SmtpSendMail(MailType *m)
{
	if (!m->szSmtpServer)
	{
		return FALSE;
	}

	CSmtp mail;

	BOOL NoErrorHasOccurred = TRUE;
	try
	{
		int i;

		// Verbindungseinstellungen
		if (!m->szSmtpServerPort)
		{ /* Benutze Standard-Ports,
		  wenn Port nicht gesetzt */
			switch (m->iSmtpSecurityType)
			{
			case USE_TLS: /*TLS*/
				m->szSmtpServerPort = 587;
				break;
			case USE_SSL: /*SSL*/
				m->szSmtpServerPort = 587;
				break;
			default: /*NO_SECURITY & DO_NOT_SET*/
				m->szSmtpServerPort = 25;
				break;
			}
		}

		if (!m->iSmtpSecurityType)
		{ /* Benutze keine Verschlaesselung,
		  wenn nicht gesetzt */
			m->iSmtpSecurityType = NO_SECURITY;
		}

		// Login
		if (m->szSmtpUser && m->szSmtpPass)
		{
			mail.SetLogin(m->szSmtpUser);
			mail.SetPassword(m->szSmtpPass);
		}

		// Servereinstellungen
		mail.SetSMTPServer(m->szSmtpServer, m->szSmtpServerPort, TRUE);
		mail.SetSecurityType(m->iSmtpSecurityType);

		// Sendeprioritaet
		if (!m->iSmtpPriority)
		{
			m->iSmtpPriority = XPRIORITY_NORMAL;
		}
		mail.SetXPriority(m->iSmtpPriority);

		// Absenderinformationen
		mail.SetSenderName(m->szSenderName);
		mail.SetSenderMail(m->szSenderAddress);
		mail.SetReplyTo(m->szReplyAddress);
		mail.SetXMailer(m->szMailerName);

		// Empfaenger
		for (i = 0; i < MailLibVectorCount(&m->RecipVec); i++)
		{ /* Fuege Empfaenger hinzu */
			mail.AddRecipient((char *)MailLibVectorGet(&m->RecipVec, i));
		}

		// Cc-Empfaenger
		for (i = 0; i < MailLibVectorCount(&m->CcRecipVec); i++) 
		{ /* Fuege Cc-Empfaenger hinzu */
			mail.AddCCRecipient((char *)MailLibVectorGet(&m->CcRecipVec, i));
		}

		// Anzahl der Bcc-Empfaenger
		for (i = 0; i < MailLibVectorCount(&m->BccRecipVec); i++) 
		{ 
			/* Fuege Bcc-Empfaenger hinzu */
			mail.AddBCCRecipient((char *)MailLibVectorGet(&m->BccRecipVec, i));
		}

		// Main-Inhalt und -Betreff
		mail.SetSubject(m->szSubject);

		if (m->szContent) 
		{
			if (m->bHTML == TRUE)
			{
				mail.m_bHTML = true;
				mail.MsgBodyHTML = m->szContent;
			} 
			else
			{
				mail.AddMsgLine(m->szContent);
			}
		}

		// Anhaenge
		for (i = 0; i < MailLibVectorCount(&m->AttachVec); i++)
		{ /* Fuege Anhaenge hinzu */
			mail.AddAttachment((char *)MailLibVectorGet(&m->AttachVec, i));
		}

		// Senden der Email
		mail.Send();
	} 
	catch (ECSmtp e)
	{
		std::cout << "MailDLL: Error: " << e.GetErrorText() << ".\n";
		NoErrorHasOccurred = FALSE;
	} 
	catch (const std::exception &e)
	{
		std::cout << "MailDLL: Error: " << e.what();
		NoErrorHasOccurred = FALSE;
	} 
	catch (...)
	{
		std::cout << "MailDLL: Undefined Error\n";
		NoErrorHasOccurred = FALSE;
	}

	if (NoErrorHasOccurred == TRUE)
	{
		std::cout << "MailDLL: Mail was send successfully.\n";
	}
	// TODO: Speicher freigeben von grMail?
	return NoErrorHasOccurred;
}

/****************************************************************************/
// Funktion: int MailLibInit(char* szSmtpServer, char* szSenderAddress)
//
// Zweck:
// Initialisiert die Mailstruktur mit Standardparametern
//
//    Parameter:
//    char* szSmtpServer,
//    char* szSenderAddress
//    char* szSenderName,
//    char* szMailerName
//
//    Return:
//    int 1 wenn OK oder 0 wenn fehlgeschlagen
/****************************************************************************/

MAIL_DLL_EXPORT int MailLibInit(char *szSmtpServer, int szSmtpPort,
	int szSmtpSecurityType, char *szSenderAddress,
	char *szSenderName, char *szMailerName)
{
	// Smtp-Server Parameter setzen
	grMail.szSmtpServer = CopyAndResize(grMail.szSmtpServer, szSmtpServer);

	grMail.szSmtpServerPort = szSmtpPort;
	grMail.iSmtpSecurityType = (SMTP_SECURITY_TYPE)szSmtpSecurityType;

	// Mail Parameter setzen
	grMail.iSmtpPriority = XPRIORITY_NORMAL;

	// Initalisierung der Empfaenger-Vektoren
	MailLibVectorInit(&(grMail.RecipVec));
	MailLibVectorInit(&(grMail.CcRecipVec));
	MailLibVectorInit(&(grMail.BccRecipVec));

	// Initialisierung des Anhangvektors
	MailLibVectorInit(&(grMail.AttachVec));

	// Absenderinformationen
	grMail.szSenderAddress =
		CopyAndResize(grMail.szSenderAddress, szSenderAddress);
	grMail.szReplyAddress = CopyAndResize(grMail.szReplyAddress, szSenderAddress);
	grMail.szSenderName = CopyAndResize(grMail.szSenderName, szSenderName);
	grMail.szMailerName = CopyAndResize(grMail.szMailerName, szMailerName);

	return TRUE;
}

/****************************************************************************/
// Funktion: int MailLibSendMail(char* szRecip, char* szSubject,
//              char* szContent, BOOL bHTML)
//
// Zweck:
// Initialisiert die Mailstruktur mit Standardparametern
//
//    Parameter:
//    char* szRecip,    Empfaenger-Addressen (loennen mit ","/";" getrennt sein
//    char* szSubject,  Betreffzeile
//    char* szContent      E-Mail-Inhalt
//  BOOL bHTML          E-Mail-Inhalt ist in HTML
//
//    Return:
//    int 1 wenn OK oder 0 wenn fehlgeschlagen
/****************************************************************************/

MAIL_DLL_EXPORT int MailLibSendMail(char *szRecip, char *szSubject,
	char *szContent, BOOL bHTML)
{
	// Empfaenger
	MailLibAddRecipients(szRecip);

	// Betreff
	grMail.szSubject = CopyAndResize(grMail.szSubject, szSubject);

	// Inhalt
	grMail.szContent = CopyAndResize(grMail.szContent, szContent);

	// Inhalt ist im HTML-Format
	grMail.bHTML = bHTML;

	// Senden der Email
	return SmtpSendMail(&grMail);
}

/****************************************************************************/
// Funktion: int MailLibSendMailByHTMLTemplateFile(char* szRecip,
//              char* szSubject, char* szTemplateFilePath, char* szSubtitle,
//              char* szListTitle, char* szList, char* szDetailTitle,
//              char* szDetailTable, char* szSignaturList)
//
// Zweck:
//    Sendet Email und setzt den Inhalt ueber ein Template mit Inhalten
//
// Parameter:
//    char* szRecip               Empfaengerliste (mit Trennzeichen getrennt:
//                                  ';' oder ',')
//    char* szSubject             Betreff der Email
//      char* szTemplateFilePath  Dateipfad zur Templatedatei
//    char* szSubtitle            Untertitel der Email (nur sichtbar
//                                  in Listenansicht)
//    char* szListTitle           Ueberschrift des Listenbereichs
//    char* szList                Listeneintraege (mit Trennzeichen
//    getrennt)
//    char* szDetailTitle         Ueberschrift des Tabellenbereichs
//    char* szDetailTable         Tabelleneintraege (mit Trennzeichen fuer
//                                  Zeilen und Zellen getrennt)
//    char* szSignaturList        Untertitel/-schrifteintraege (mit Trennzeichen
//                                  getrennt)
//
// Return:
//    int 1 wenn OK oder 0 wenn fehlgeschlagen
/****************************************************************************/

MAIL_DLL_EXPORT int MailLibSendMailByHTMLTemplateFile(
	char *szRecip, char *szSubject, char *szTemplateFilePath, char *szSubtitle,
	char *szListTitle, char *szList, char *szDetailTitle, char *szDetailTable,
	char *szSignaturList)
{
	char *szContent = PopulateTemplateByContent(
		szTemplateFilePath, szSubtitle, szListTitle, szList, szDetailTitle,
		szDetailTable, szSignaturList);
	bool res = MailLibSendMail(szRecip, szSubject, szContent, TRUE);
	free(szContent);
	return res;
}

/****************************************************************************/
// Funktion: char* PopulateTemplateByContent(char* szTemplateFilePath, char*
// szSubtitle, char* szListTitle, char* szList, char* szDetailTitle, char*
// szDetailTable, char* szSignaturList)
//
// Zweck:
//    Bestaeckt ein Template mit Inhalten (Platzhalter sind in 'MailLib.h'
//    vordefiniert)
//
// Parameter:
//    char* szTemplateFilePath  Dateipfad zur Templatedatei
//    char* szSubtitle          Untertitel der Email (nur sichtbar in
//                                Listenansicht)
//    char* szListTitle         Ueberschrift des Listenbereichs
//    char* szList              Listeneintraege (mit Trennzeichen getrennt)
//    char* szDetailTitle       Ueberschrift des Tabbelenbereichs
//    char* szDetailTable       Tabelleneintraege (mit Trennzeichen fuer Zeilen
//                                und Zellen getrennt)
//    char* szSignaturList      Untertitel/-schrifteintraege (mit Trennzeichen
//                                getrennt)
//
// Return:
//    Beschriebener String mit gesamten Inhalt
/****************************************************************************/

MAIL_DLL_EXPORT char *
	PopulateTemplateByContent(char *szTemplateFilePath, char *szSubtitle,
	char *szListTitle, char *szList, char *szDetailTitle,
	char *szDetailTable, char *szSignaturList)
{
	int i;
	std::stringstream tmp;
	CVECTOR tmpVec;

	if (FileExists(szTemplateFilePath) == 0)
	{
		return NULL;
	}
	/**** Lesen der Template-Datei ****/
	std::ifstream f(szTemplateFilePath);
	tmp << f.rdbuf();
	std::string sTemplateContent = tmp.str();
	tmp.str("");

	/************* Titel **************/
	// Setzen des Untertitels im Template
	ReplaceInString(sTemplateContent, TEMPLATE_SUBTITLE_LOOKUP, szSubtitle);

	// Setzen des Titels ueber der Liste
	ReplaceInString(sTemplateContent, TEMPLATE_LIST_TITLE_LOOKUP, szListTitle);

	// Setzen des Titels ueber der Tabelle
	ReplaceInString(sTemplateContent, TEMPLATE_DETAIL_TITLE_LOOKUP, szDetailTitle);

	/************** Liste *************/
	// Aufteilung des Listenstrings in Vektor
	MailLibVectorInit(&tmpVec);
	StringToVector(&tmpVec, szList, TEMPLATE_LIST_DELIMITER);
	// Setzen der Listen-Eintraege im Template
	ReplaceInString(sTemplateContent, TEMPLATE_LIST_LOOKUP, VectorToHTML(&tmpVec, "<li>", "</li>\n"));
	MailLibVectorFree(&tmpVec);

	/************ Tabelle *************/
	CVECTOR DetailVecRows;
	MailLibVectorInit(&DetailVecRows);
	// Aufteilung der Zeilen des Detailstrings in Vektor
	StringToVector(&DetailVecRows, szDetailTable, TEMPLATE_TABLE_ROW_DELIMITER);

	for (i = 0; i < MailLibVectorCount(&DetailVecRows); i++)
	{
		MailLibVectorInit(&tmpVec);
		// Aufteilung der Spalten des Detail-Tabellen-Zeilen-Vektors in Vektor
		StringToVector(&tmpVec,
			(char *)MailLibVectorGet(&DetailVecRows, i),
			TEMPLATE_TABLE_COLUMN_DELIMITER);
		// Schreibe Zeilen
		tmp << "<tr>" << VectorToHTML(&tmpVec, "<td>", "</td>\n") << "</tr>\n";
		MailLibVectorFree(&tmpVec);
	}
	ReplaceInString(sTemplateContent, TEMPLATE_DETAIL_LOOKUP, tmp.str());
	MailLibVectorFree(&DetailVecRows);

	/************ Signatur ************/
	// Setzen der Signatur-Eintraege
	// Aufteilung des Listenstrings in Vektor
	MailLibVectorInit(&tmpVec);
	StringToVector(&tmpVec, szSignaturList, TEMPLATE_SIGNATUR_DELIMITER);
	ReplaceInString(sTemplateContent,
		TEMPLATE_SIGNATURE_LOOKUP,
		VectorToHTML(&tmpVec, "<tr>\n<td>", "</td>\n</tr>\n"));
	MailLibVectorFree(&tmpVec);

	return strdup(sTemplateContent.c_str());
}

/****************************************************************************/
// Funktion: int MailLibAddAttachment(char* szFilePath)
//
// Zweck:
//      fuegt Dateipfad an den Anhangpfadevektor an
//
// Parameter:
//     char* szFilePath
//
// Return:
//     int 1 wenn OK oder 0 wenn fehlgeschlagen
/****************************************************************************/

MAIL_DLL_EXPORT int MailLibAddAttachment(char *szFilePath)
{
	// Wenn exitiert, an Anhangpfadevektor anhaengen
	bool exists = FileExists(szFilePath);
	if (exists == true)
	{
		MailLibVectorAdd(&(grMail.AttachVec), szFilePath);
	}
	return (int)exists;
}

/****************************************************************************/
// Funktion: void MailLibClearAttachment()
//
// Zweck:
//      Leert Anhangpfadevektor
//
/****************************************************************************/

MAIL_DLL_EXPORT void MailLibClearAttachment()
{
	// Anhangpfadevektor leeren
	MailLibVectorFree(&(grMail.AttachVec));
	MailLibVectorInit(&(grMail.AttachVec));
}

// Hilfsfunktion: Existenz der Datei mit Dateinamen
bool FileExists(const char *szFilePath)
{
	struct stat buffer;
	return (stat(szFilePath, &buffer) == 0);
}

/****************************************************************************/
// Funktion: void MailLibSetAuthentification(char* szUsername, char*
//              szUserPassword)
//
// Zweck:   Setzt Authentifizierungsparameter
//
// Parameter:
//    char* szUsername,
//    char* szUserPassword
//
// Return:
//    int 1 wenn OK oder 0 wenn fehlgeschlagen
/****************************************************************************/

MAIL_DLL_EXPORT void MailLibSetAuthentification(char *szUsername,
	char *szUserPassword)
{
	// Speicher setzen
	grMail.szSmtpUser = CopyAndResize(grMail.szSmtpUser, szUsername);
	grMail.szSmtpPass = CopyAndResize(grMail.szSmtpPass, szUserPassword);
}

/****************************************************************************/
// Funktion: int MailLibAddRecipients(char* szRecipients)
//           int MailLibAddCcRecipients(char* szCcRecipients)
//           int MailLibAddBccRecipients(char* szBccRecipients)
//
// Zweck:
//    Teilt einen String auf (ueber Trennzeichen ',' oder ';') und setzt
//      entsprechende Empfaengervektoren
//
/****************************************************************************/

MAIL_DLL_EXPORT void MailLibAddRecipients(char *szRecipients)
{
	MailLibVectorInit(&(grMail.RecipVec));
	StringToVector(&(grMail.RecipVec), szRecipients, ",;");
}

MAIL_DLL_EXPORT void MailLibAddCcRecipients(char *szCcRecipients)
{
	MailLibVectorInit(&(grMail.CcRecipVec));
	StringToVector(&(grMail.CcRecipVec), szCcRecipients, ",;");
}

MAIL_DLL_EXPORT void MailLibAddBccRecipients(char *szBccRecipients)
{
	MailLibVectorInit(&(grMail.BccRecipVec));
	StringToVector(&(grMail.BccRecipVec), szBccRecipients, ",;");
}

/****************************************************************************/
// Funktion: void MailLibClearAllRecipients()
//
// Zweck:
//    Initialisiert alle Empfaengervektoren
/****************************************************************************/

MAIL_DLL_EXPORT void MailLibClearAllRecipients()
{
	MailLibVectorFree(&(grMail.RecipVec));
	MailLibVectorFree(&(grMail.CcRecipVec));
	MailLibVectorFree(&(grMail.BccRecipVec));
	MailLibVectorInit(&(grMail.RecipVec));
	MailLibVectorInit(&(grMail.CcRecipVec));
	MailLibVectorInit(&(grMail.BccRecipVec));
}

/****************************************************************************/
// Funktion: void StringToVector(CVECTOR* pVec, char* szString, char*
// szDelimiter)
//
// Zweck:
//    Parst einen String und speichert die einzelnen Teilstrings in
//      einem c-Vector
//
// Parameter:
//    CVECTOR* pVec      Pointer auf den Vector, in den die geteilten Strings
//                         abgelegt werden sollen
//    char* szString     Adressen-String
//    char* szDelimiter  Trennzeichen (z.B.: ",;")
//
/****************************************************************************/

MAIL_DLL_EXPORT void StringToVector(CVECTOR *pVec, char *szString,
	char *szDelimiter)
{
	int len;

	// Vektor initialisieren
	MailLibVectorInit(pVec);

	// Temporaerer Stringkopie
	len = strlen(szString) + 1;
	char *str = (char *)malloc(len * sizeof(char));
	str[0] = '\0';
	strcpy_s(str, len, szString);

	// String aufteilen
	char *tmp = strtok(str, szDelimiter);
	while (tmp != NULL)
	{
		MailLibVectorAdd(pVec, tmp);
		tmp = strtok(NULL, szDelimiter);
	}
	// "str" wird hier nicht delokiert, weil die Pointer in den Vektor geschrieben
	// sind
}

/****************************************************************************/
// Funktionen: Vektor-Hilfs-Funktionen
//
/****************************************************************************/

MAIL_DLL_EXPORT void MailLibVectorInit(CVECTOR *v)

{ vector_init(v); };
MAIL_DLL_EXPORT int MailLibVectorCount(CVECTOR *v)

{ return vector_count(v); };
MAIL_DLL_EXPORT void MailLibVectorAdd(CVECTOR *v, void *e)

{ vector_add(v, e); };
MAIL_DLL_EXPORT void MailLibVectorSet(CVECTOR *v, int i, void *e)

{ vector_set(v, i, e); };
MAIL_DLL_EXPORT void *MailLibVectorGet(CVECTOR *v, int i)

{ return vector_get(v, i); };
MAIL_DLL_EXPORT void *MailLibVectorDelete(CVECTOR *v, int i)

{ return vector_delete(v, i); };
MAIL_DLL_EXPORT void MailLibVectorFree(CVECTOR *v)

{ vector_free(v); };

/****************************************************************************/
// Funktion: char* MailLibReadFromFile(char* fileName)
//
// Zweck:
//    Liest eine Datei ein und gibt den Inhalt als String zurueck
//
// Parameter:
//    char* fileName
//
// Return:
//    Dateiinhalt
/****************************************************************************/

MAIL_DLL_EXPORT char *MailLibReadFromFile(char *fileName)
{
	std::ifstream f(fileName);
	std::stringstream buffer;
	buffer << f.rdbuf();
	string str = buffer.str();
	return strdup(str.c_str()); // Speicher muss mit "free" freigegeben werden
	// return (char*) strdup(str.c_str());
}

/****************************************************************************/
// Funktion: int MailLibCopyFile(char* szSrcFile, char* szDestFile)
//
// Zweck:
// Kopiert Quellfile mit Pfad nach Ziel
//
// Parameter:
//    char* szSrcFile,
//    char* szDestFile
//
//    Return:
//    int 1 wenn OK oder 0 wenn fehlgeschlagen
/****************************************************************************/

MAIL_DLL_EXPORT int MailLibCopyFile(char *szSrcFile, char *szDestFile)
{
	return CopyFile(szSrcFile, szDestFile, FALSE);
}

/****************************************************************************/
// Funktion: void ReplaceInString( std::string &subject,
//                           const std::string &search,
//                           const std::string &replace)
//
// Zweck: Ersetzt in einem String einen Suchstring durch einen Ersetzungsstring
//
// Parameter:
//      char* search
//      char* replace
//      char* str
//
// Rueckgabe: Pointer auf neuen String (der mit "free" wieder freigegeben
//             werden muss)
/****************************************************************************/

void ReplaceInString(std::string &subject, const std::string &search,
	const std::string &replace)
{
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos)
	{
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
}

/****************************************************************************/
// Funktion: char* VectorToHTML(CVECTOR* Vec, char* szStartTag, char* szEndTag)
//
// Zweck:
//    Schließt alle Eintraege eines Vektors in HTML-Tags ein und fuegt sie zu
//      einem String zusammen
//
//    Parameter:
//        CVECTOR* Vecchar* szEndTag
//        char*    szStartTag
//        char*    szEndTag
//
//    Return:
//        Zusammengesetzter String
//
/****************************************************************************/

std::string VectorToHTML(CVECTOR *Vec, 
	const std::string &szStartTag,
	const std::string &szEndTag)
{
	std::stringstream res;

	for (int i = 0; i < MailLibVectorCount(Vec); i++)
	{ /* Fuege Vektor-Elemente hinzu */
		res << szStartTag << (char *)MailLibVectorGet(Vec, i) << szEndTag;
	}
	return res.str();
}

/****************************************************************************/
// Funktion: void* CopyAndResize(void* dest, void* orig)
//
// Zweck: Kopiert eine Variable an einen Speicherbereich
//
// Parameter:
//    void* dest
//    void* orig
//
// Return:
//    Pointer zur Variable
//
/****************************************************************************/
char *CopyAndResize(char *dest, const char *orig)
{
	int newSize = strlen(orig) * sizeof(char) + 1;
	dest = (char *)realloc(dest, newSize);
	if (dest == NULL)
	{
		return NULL;
	}
	memcpy(dest, orig, newSize);
	return dest;
}

/****************************************************************************/
// Funktion: int SecSeit1970( int nDay, int nMonth, int nYear, int nHour, int
//                              nMinute)
//
// Zweck: Funktion die aus Datum/Uhrzeit die vergangenen Sekunden seit dem 1.1.1970
//          zurueckliefert
//
// Return:
//    int 1 wenn OK oder 0 wenn fehlgeschlagen
/****************************************************************************/

MAIL_DLL_EXPORT int SecSeit1970(int nDay, int nMonth, int nYear, int nHour,
	int nMinute)
{
	int nSekunden;
	struct tm TM;

	TM.tm_sec = 0;             /* Sekunden - [0,61] */
	TM.tm_min = nMinute;       /* Minuten - [0,59] */
	TM.tm_hour = nHour;        /* Stunden - [0,23] */
	TM.tm_mday = nDay;         /* Tag des Monats - [1,31] */
	TM.tm_mon = nMonth - 1;    /* Monat im Jahr - [0,11] */
	TM.tm_year = nYear - 1900; /* Jahr seit 1900 */
	// TM.tm_wday;     /* Tage seit Sonntag (Wochentag) - [0,6] */
	// TM.tm_yday;     /* Tage seit Neujahr (1.1.) - [0,365] */
	TM.tm_isdst = -1; /* Sommerzeit-Flag */

	nSekunden = (int)mktime(&TM);

	return nSekunden;
}

/****************************************************************************/
// Funktion: int VerInfo(char*szInfo)
//
// Zweck: liefert eine Versionsinfo aus der entsprechenden Ressource in den
//          uebergebenen String zurueck
//
// Parameter:
//    char* szInfo  vom Aufrufer ausreichend gross (mind. 32 Zeichen!!!)
//                    bereitzustellen, hier wird die  Versionsinfo reinkopiert
//
// Return:
//    negativ oder 0=Fehler  1=TRUE=ok
/****************************************************************************/

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

MAIL_DLL_EXPORT int VerInfo(char *szInfo)
{
	TCHAR szDllPath[MAX_PATH];
	//    TCHAR szVerInfo[MAX_PATH] = "CABURDLL: ";
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
		char *pcInfoBuffer = NULL;

		if ((dwInfoSize = GetFileVersionInfoSize(szDllPath, &dwDummy)) != 0)
		{
			VS_FIXEDFILEINFO *pFixedFile = NULL;
			UINT wInfoLen;

			pcInfoBuffer = (char *)malloc(dwInfoSize);

			if (GetFileVersionInfo(szDllPath, 0UL, dwInfoSize, pcInfoBuffer) != 0)
			{
				LPVOID pvInfo;

				// Description
				if (VerQueryValue(pcInfoBuffer, TEXT("\\VarFileInfo\\Translation"),
					&pvInfo, &wInfoLen) != 0)
				{
					TCHAR szFormat[80], *szData = NULL;
					WORD wCodePage = LOWORD(*(DWORD *)pvInfo);
					WORD wLangID = HIWORD(*(DWORD *)pvInfo);

					sprintf_s(
						(char *)szFormat, 80,
						(const char *)TEXT("StringFileInfo\\%04X%04X\\FileDescription"),
						(char *)wCodePage, (char *)wLangID);
					if (VerQueryValue(pcInfoBuffer, szFormat, (void **)&szData,
						&wInfoLen) != 0)
					{
						strcpy_s((char *)szDescription, MAX_PATH, (char *)szData);
						bDescriptionRead = TRUE;
					}
				}

				// Versionsinfo
				if (VerQueryValue(pcInfoBuffer, TEXT("\\"), (void **)&pFixedFile,
					&wInfoLen) != 0)
				{
					short nVersion1;
					short nVersion2;
					short nVersion3;
					short nVersion4;

					// codiert als HEX in HIWORD und LOWWORD
					dwMajorVersion = pFixedFile->dwFileVersionMS;
					dwMinorVersion = pFixedFile->dwFileVersionLS;

					// extrahieren in Variablen
					nVersion1 = HIWORD(dwMajorVersion);
					nVersion2 = LOWORD(dwMajorVersion);
					nVersion3 = HIWORD(dwMinorVersion);
					nVersion4 = LOWORD(dwMinorVersion);

					sprintf_s((char *)szVersion, MAX_PATH, (char *)TEXT("%d.%d.%d.%d "),
						nVersion1, nVersion2, nVersion3, nVersion4);

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

	// Infos zusammenstellen
	// erst mal leer
	szVerInfo[0] = 0;

	if (bVersionRead)
		strcat_s((char *)szVerInfo, MAX_PATH, (char *)szVersion);

	if (bDescriptionRead)
		strcat_s((char *)szVerInfo, MAX_PATH, (char *)szDescription);

	strcpy_s((char *)szInfo, RETURNSIZE, (char *)szVerInfo);

	if (bVersionRead == TRUE && bDescriptionRead == TRUE)
		return TRUE;
	else
		return FALSE;
}

/****************************************************************************/
// Funktion: int MailLibDirInfo(char*szInfo)
//
// Zweck:  liefert das eigene DLL-Directory in den uebergebenen String zurueck
//
// Parameter:
//    char* szInfo  vom Aufrufer ausreichend gross (mind. 255 Zeichen!!!)
//                    bereitzustellen, hier wird der gesamte Pfad der DLL
//                    reinkopiert
//
// Return:
//    negativ oder 0=Fehler  1=TRUE=ok
/****************************************************************************/
// Funktion die das eigene DLL-Directory zurueckliefert

#define BUFSIZE MAX_PATH

MAIL_DLL_EXPORT int MailLibDirInfo(char *szInfo)
{
	HMODULE hDll;

	if (szInfo == NULL)
		return -1;

	/* Liefert nur den Ordner des Aufrufenden Prozesses zurueck !
	TCHAR Buffer[BUFSIZE];
	DWORD dwRet;
	dwRet = GetCurrentDirectory(BUFSIZE, Buffer);
	if( dwRet == 0 )
	sprintf_s(Buffer, RETURNSIZE, TEXT("GetCurrentDirectory failed (%d)\n"),
	GetLastError());
	strcpy_s(szInfo, RETURNSIZE, Buffer);*/

	hDll = GetModuleHandle(MAILLIB_DLL_NAME);
	if (hDll == NULL)
	{
		strcpy_s(szInfo, RETURNSIZE, (char *)TEXT("NO HANDLE"));
		return -2;
	} 
	else
	{
		if (GetModuleFileName(hDll, (LPSTR)szInfo, MAX_PATH) == 0)
		{
			strcpy_s(szInfo, RETURNSIZE, (char *)TEXT("NO FileName"));
			return -3;
		}
	}

	return TRUE;
}

/****************************************************************************/
// Funktion: int MailLibDirInfoSize(void)
//
// Zweck:  liefert die Graeaee des Strings der das eigene DLL-Directory haelt zurueck
//
// Parameter:
//    void
//
// Return:
//    int Graeaee in Zeichen oder 0 wenn fehlgeschlagen
/****************************************************************************/
MAIL_DLL_EXPORT int MailLibDirInfoSize(void)
{
	char Buffer[RETURNSIZE];

	if (MailLibDirInfo(Buffer) == TRUE)
		return (int)strlen(Buffer);
	else
		return FALSE;
}
