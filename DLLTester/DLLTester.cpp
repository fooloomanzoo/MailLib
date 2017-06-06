/*****************************************************************************/
/* DLLTESTER.cpp : Konsolenanwendung zum Test der Funktionen in DLL.         */
/*                                                                           */
/* Compiler: Visual Studio 2010 Compiler                                     */
/*                                                                           */
/* erstellt durch Werner Huerttlen w.huerttlen@fz-juelich.de (22.11.2007)    */
/* erweitert durch Johannes Brautzsch j.brautzsch@fz-juelich.de (01.03.2017) */
/*****************************************************************************/
#include "stdafx.h"
#include <iostream>
#include <sstream>
#include <string>
#include <windows.h>
#include <conio.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>

// DEFINE Verbose Modus
// #define MAIL_DLL_VERBOSE

namespace std
{
#if defined UNICODE || defined _UNICODE
  typedef wstring tstring;
  typedef wstringbuf tstringbuf;
  typedef wstringstream tstringstream;
  typedef wostringstream tostringstream;
  typedef wistringstream tistringstream;
  typedef wfilebuf tfilebuf;
  typedef wfstream tfstream;
  typedef wifstream tifstream;
  typedef wofstream tofstream;
#else  // defined UNICODE || defined _UNICODE
  typedef string tstring;
  typedef stringbuf tstringbuf;
  typedef stringstream tstringstream;
  typedef ostringstream tostringstream;
  typedef istringstream tistringstream;
  typedef filebuf tfilebuf;
  typedef fstream tfstream;
  typedef ifstream tifstream;
  typedef ofstream tofstream;
#endif // defined UNICODE || defined _UNICODE
} // namespace std

#include "../MailLib/MailDll.h"

// Versionsnummer
#define VERSION "DLLTESTER.EXE V1.2 01.03.2017\n"

char szVerInfo[512]; // String um die Versionsinfo der DLL zu halten
char szDirInfo[512]; // String um die Versionsinfo der DLL zu halten

int _tmain(int argc, _TCHAR *argv[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);


	/*****************************************************************************/
	/*                               Versionsinfos                               */
	/*****************************************************************************/
	// Version des Testprogramms
	std::cout << VERSION;

	// Versionsinfo aus Ressource der DLL lesen
	VerInfo(szVerInfo);
	std::cout << "DLL: VerInfo: " << szVerInfo << "\n";

	// Pfad der DLL ausgeben
	std::cout << "DLL: MailLibDirInfoSize: " << MailLibDirInfoSize() << "\n";
	MailLibDirInfo(szDirInfo);
	std::cout << "DLL: MailLIbDirInfo: " << szDirInfo << "\n";

	bool bSuccess = false;

	/*****************************************************************************/
	/*                               TESTFAELLE                                  */
	/*****************************************************************************/
	bool BENUTZE_DIREKTE_FUNKTIONEN = true;
	bool SETZE_MAIL_STRUKTUR_SELBST = true;

	bool SENDE_EMAIL_UEBER_HTML_TEMPLATE = true;
	bool SENDE_EINFACHE_HTML_EMAIL = true;
	bool SENDE_TEXT_EMAIL = true;

	bool PORT_TEST = true;
	bool VEKTOR_TEST = false;

	/*****************************************************************************/
	/*                            TESTDEFINITIONEN                               */
	/*****************************************************************************/

	char *SMPTSERVER = TEXT("mailrelay.fz-juelich.de");
	int  SMPTPORT = 587;
	SmtpSecurityType SMPTSECURITY = USE_TLS;
	char *MAILERNAME = TEXT("MailLibDll-Tester");
	SmptPriority PRIORITY = XPRIORITY_NORMAL;

	char *SENDER = TEXT("j.brautzsch@fz-juelich.de");
	char *SENDERNAME = TEXT("Johannes Brautzsch");
	char *USER = TEXT("j.brautzsch");
	char *PASS = NULL;

	//char *SMPTSERVER = TEXT("smtp.gmail.com");
	//char *SENDER = TEXT("johannes.brautzsch@gmail.com");
	//char *USER = TEXT("johannes.brautzsch@gmail.com");

	char *CONTENT = TEXT("This is a Test-Email!");
	char *CONTENT_FILE = TEXT("..\\SampleContent\\Test-HTML-Inhalt.html");
	char *CONTENT_TEMPL_FILE = TEXT("..\\SampleContent\\Test-HTML-Template.html");

	char *T_SUBTITLE = TEXT("Das is ein beliebiger Vorschau-Text");
	char *T_LIST_TITLE = TEXT("Hier stehen die Listeneintraege:");
	char *T_LIST = TEXT("Eintrag 1; Eintrag 2");
	char *T_DETAIL_TITLE = TEXT("Hier befindet sich die Tabelle mit den Details:");
	char *T_DETAIL = TEXT("Zeile 1 Spalte 1;Zeile 1 Spalte 2;Zeile 1 Spalte 3|Zeile 2 Spalte 1;Zeile 2 Spalte 2;Zeile 2 Spalte 3;Zeile 2 Spalte 4");
	char *T_SIGNATURE = TEXT("Your Name;Your Street;Your Tel;Your Email");

	char *ATTACHMENT_PATH = TEXT("..\\SampleContent\\Test-Anhang.csv");
	char *SUBJECT = TEXT("Test Email");
	char *RECIPIENTS = TEXT("j.brautzsch@fz-juelich.de;johannes.brautzsch@gmail.com");

	// Passwort einlesen
	std::string tmp;
	std::cout << SMPTSERVER << " " << SMPTPORT << std::endl << SENDER << std::endl << USER << std::endl <<  "Passwort: ";
	char ch = _getch();
	while (ch != 13) { // Character 13 ist Enter
		tmp.push_back(ch);
		std::cout << '*'; // Versteckte Ausgabe
		ch = _getch();
	}
	std::cout << std::endl;
	PASS = strdup(tmp.c_str());


	/*****************************************************************************/
	/*                   SENDEN PER UMSCHLIESSENDEN FUNKTIONEN                   */
	/*****************************************************************************/
	if (BENUTZE_DIREKTE_FUNKTIONEN)
	{
		std::cout << "DLL-Test: SENDEN PER UMSCHLIESSENDEN FUNKTIONEN (MailLibSendMail)" << std::endl;
		// ---- Server Initialisieren -----
		MailLibInit(SMPTSERVER, SMPTPORT, SMPTSECURITY, SENDER, SENDERNAME, MAILERNAME);
		MailLibSetAuthentification(USER, PASS);

		// ---- Anhang -----
		if (ATTACHMENT_PATH) 
		{
			MailLibAddAttachment(ATTACHMENT_PATH);
		}

		if (SENDE_EINFACHE_HTML_EMAIL) 
		{   // HTML-Email
			// Datei einlesen, Setzen & Senden
			std::cout << "DLL-Test: HTML-Email einlesen, Setzen & Senden (MailLibSendMail)" << std::endl;
			bSuccess = MailLibSendMail(RECIPIENTS, SUBJECT, MailLibReadFromFile(CONTENT_FILE), TRUE);			
			std::cout << "DLL-Test:" << (bSuccess ? "" : "nicht ") << "erfolgreich" << std::endl << std::endl;
			bSuccess = false;
		} 
		if (SENDE_EMAIL_UEBER_HTML_TEMPLATE) 
		{   // Email mit HTML-Template
			// Setzen & Senden
			std::cout << "DLL-Test: HTML-Template einlesen, Setzen & Senden (MailLibSendMailByHTMLTemplateFile)" << std::endl;
			bSuccess = MailLibSendMailByHTMLTemplateFile(RECIPIENTS, SUBJECT, CONTENT_TEMPL_FILE, T_SUBTITLE, T_LIST_TITLE, 
				                                           T_LIST, T_DETAIL_TITLE, T_DETAIL, T_SIGNATURE);
			std::cout << "DLL-Test: " << (bSuccess ? "" : "nicht ") << "erfolgreich" << std::endl << std::endl;
			bSuccess = false;
		} 
		if (SENDE_TEXT_EMAIL) 
		{   // Text-Email
		    // Setzen & Senden
			std::cout <<  "DLL-Test: Text-Email Setzen & Senden (MailLibSendMail)" << std::endl;
			bSuccess = MailLibSendMail(RECIPIENTS, SUBJECT, CONTENT, TRUE);			
			std::cout << "DLL-Test: " << (bSuccess ? "" : "nicht ") << "erfolgreich" << std::endl << std::endl;
			bSuccess = false;
		}
	}


/*****************************************************************************/
/*                      DIREKTES SETZEN DER MAIL-STRUKTUR                    */
/*****************************************************************************/
	if (SETZE_MAIL_STRUKTUR_SELBST)
	{   // Beginn der Initialierung der Mail-Parameter-Struktur
		// Struktur anlegen
		std::cout << "DLL-Test: DIREKTES SETZEN DER MAIL-STRUKTUR (SmtpSendMail)" << std::endl;
		MailType grMail;

		// Empfaenger
		MailLibVectorInit(&(grMail.RecipVec));
		StringToVector(&(grMail.RecipVec), RECIPIENTS, ";,");
		// Adresse der Kopienempfaenger
		MailLibVectorInit(&(grMail.CcRecipVec));
		// Adresse der Blind-Kopienempfaenger
		MailLibVectorInit(&(grMail.BccRecipVec));
		// Smtp-Server
		grMail.szSmtpServer = SMPTSERVER;
		grMail.szSmtpServerPort = SMPTPORT;
		grMail.iSmtpSecurityType = SMPTSECURITY;
		grMail.iSmtpPriority = PRIORITY;
		// Authentifizierung
		grMail.szSmtpUser = USER;
		grMail.szSmtpPass = PASS;
		// Absender
		grMail.szSenderAddress = SENDER;
		grMail.szSenderName = SENDERNAME;
		grMail.szReplyAddress = SENDER;
		// Mailer
		grMail.szMailerName = MAILERNAME;
		// Inhalt und Betreff
		grMail.szSubject = SUBJECT;
		// Anhang
		if (ATTACHMENT_PATH)
		{   // Pfad der Datei
			MailLibVectorInit(&(grMail.AttachVec));
			MailLibVectorAdd(&(grMail.AttachVec), ATTACHMENT_PATH);
		}

		// ---- Setzen des Inhalts ----
		if (SENDE_EINFACHE_HTML_EMAIL)
		{   // HTML-Email
			std::cout << "DLL-Test: HTML-Email (SmtpSendMail)" << std::endl;
			grMail.szContent = MailLibReadFromFile(CONTENT_FILE);
			grMail.bHTML = true;
			// Senden
			bSuccess = SmtpSendMail(&grMail);			
			std::cout << "DLL-Test: " << (bSuccess ? "" : "nicht ") << "erfolgreich" << std::endl << std::endl;
			bSuccess = false;
		}
		if (SENDE_EMAIL_UEBER_HTML_TEMPLATE)
		{   // Email mit HTML-Template
			std::cout << "DLL-Test: Email mit HTML-Template (PopulateTemplateByContent, SmtpSendMail)" << std::endl;
			grMail.bHTML = TRUE;
			grMail.szContent = PopulateTemplateByContent(CONTENT_TEMPL_FILE,
				T_SUBTITLE,
				T_LIST_TITLE,
				T_LIST,
				T_DETAIL_TITLE,
				T_DETAIL,
				T_SIGNATURE);
			if (grMail.szContent != NULL)
			{   // Senden
				bSuccess = SmtpSendMail(&grMail);
			}
			std::cout << "DLL-Test: " << (bSuccess ? "" : "nicht ") << "erfolgreich" << std::endl << std::endl;
			bSuccess = false;
		}
		if (SENDE_TEXT_EMAIL)
		{   // Text-Email
			std::cout << "DLL-Test: Text-Email (SmtpSendMail)" << std::endl;
			// Textinhalt
			grMail.szContent = CONTENT;
			// Senden
			bSuccess = SmtpSendMail(&grMail);
			std::cout << "DLL-Test: " << (bSuccess ? "" : "nicht ") << "erfolgreich" << std::endl << std::endl;
			bSuccess = false;
		}
	}


/*****************************************************************************/
/*                               PORT_TESTS                                  */
/*****************************************************************************/
	if (PORT_TEST)
	{
		std::cout << "DLL-Test: Port Tests \n--> Ports: \n25 (unverschluesselt), \n3333 (Fehler beim Senden erwartet), \n465 (Standard SMTP-SSL), \n587 (Standard SMTP-TLS)" << std::endl;
		std::stringstream ss;
		std::string sContent;
		int ports[4] = {25, 3333, 465, 587};
		SmtpSecurityType securityTypes[4] = { NO_SECURITY, USE_TLS, USE_SSL, DO_NOT_SET };
		char* securityTypeTexts[4] = { TEXT("NO_SECURITY"), TEXT("USE_TLS"), TEXT("USE_SSL"), TEXT("DO_NOT_SET") };

		// Beginn der Initialierung der Mail-Parameter-Struktur
		// Struktur anlegen
		MailType grMail;

		// Empfaenger
		MailLibVectorInit(&(grMail.RecipVec));
		StringToVector(&(grMail.RecipVec), RECIPIENTS, ";,");
		// Adresse der Kopienempfaenger
		MailLibVectorInit(&(grMail.CcRecipVec));
		// Adresse der Blind-Kopienempfaenger
		MailLibVectorInit(&(grMail.BccRecipVec));
		// Smtp-Server
		grMail.szSmtpServer = SMPTSERVER;
		grMail.iSmtpPriority = PRIORITY;
		// Authentifizierung
		grMail.szSmtpUser = USER;
		grMail.szSmtpPass = PASS;
		// Absender
		grMail.szSenderAddress = SENDER;
		grMail.szSenderName = SENDERNAME;
		grMail.szReplyAddress = SENDER;
		// Mailer
		grMail.szMailerName = MAILERNAME;
		// Inhalt und Betreff
		grMail.szSubject = SUBJECT;
		// Anhang
		if (ATTACHMENT_PATH)
		{   // Pfad der Datei
			MailLibVectorInit(&(grMail.AttachVec));
			MailLibVectorAdd(&(grMail.AttachVec), ATTACHMENT_PATH);
		}
		
		for (byte i = 0; i < 4; i++)
		{
			grMail.szSmtpServerPort = ports[i];
			for (byte j = 0; j < 4; j++)
			{
				grMail.iSmtpSecurityType = securityTypes[j];
				ss << "DLL-Test: Test-Email on Port: " << ports[i] << " (" << securityTypeTexts[j] << ")";
				sContent = ss.str();
				grMail.szContent = strdup(sContent.c_str());
				std::cout << grMail.szContent << std::endl;
				// Senden
				bSuccess = SmtpSendMail(&grMail);
				std::cout << "DLL-Test: " << (bSuccess ? "" : "nicht ") << "erfolgreich" << std::endl << std::endl;
				bSuccess = false;
				ss.str("");
			}
		}
	}   


/*****************************************************************************/
/*                         Teile String in C-Vector                          */
/*****************************************************************************/
    if (VEKTOR_TEST)
    {
		std::cout << "DLL-Test: Vektor Tests (StringToVector, MailLibVectorInit)" << std::endl;
	    char *TestString = TEXT("c.onsuela@fz-juelich.de, m.holtkoetter@fz-juelich.de; markus.holtkoetter@dialup.fh - aachen.de, abc@example.com");
	    char *TestDelimiter = TEXT(";,");

        CVECTOR TestVector;
        MailLibVectorInit(&TestVector);
	    StringToVector(&TestVector, TestString, TestDelimiter);

	    std::cout << "\nDLL-Test: Teststring: \"" << TestString << "\" wird aufgeteilt:" << std::endl;

	    for (int i = 0; i < MailLibVectorCount(&TestVector); i++)
        { /* Ausgabe der Vektor-Elemente */
            std::cout << (char *)MailLibVectorGet(&TestVector, i) << std::endl;
	    }
    }

/*****************************************************************************/
/*                                   Ende                                    */
/*****************************************************************************/
    std::cout << "DLL-Test: Fertig, bitte Taste druecken... " << std::endl;
    getchar();

    return 0;
}
