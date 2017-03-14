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
#include <string>
#include <windows.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>

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

/*****************************************************************************/
/*                            TESTDEFINITIONEN                               */
/*****************************************************************************/
#define BENUTZE_DIREKTE_FUNKTIONEN
//#define SETZE_MAIL_STRUKTUR_SELBST
//#define VEKTOR_TEST

#define SENDE_EMAIL_UEBER_HTML_TEMPLATE
//#define SENDE_EINFACHE_HTML_EMAIL
//#define SENDE_TEXT_EMAIL

#define SMPTSERVER   TEXT("mailrelay.fz-juelich.de")
#define SMPTPORT     587
#define SMPTSECURITY USE_TLS
#define MAILERNAME   TEXT("MailLibDll-Tester")
#define PRIORITY     XPRIORITY_NORMAL

#define SENDER     TEXT("j.brautzsch@fz-juelich.de")
#define SENDERNAME TEXT("Johannes Brautzsch")
#define USER       TEXT("j.brautzsch")
#define PASS       TEXT("xxx")

#define CONTENT            TEXT("Das ist eine Test-Email!")
#define CONTENT_FILE       TEXT("..\\SampleContent\\Test-HTML-Inhalt.html")
#define CONTENT_TEMPL_FILE TEXT("..\\SampleContent\\Test-HTML-Template.html")

#define T_SUBTITLE     TEXT("Dies ist ein Vorschautext")
#define T_LIST_TITLE   TEXT("Es stehen zur Zeit folgende Alarme an:")
#define T_LIST         TEXT("Alarm1;Alarm2")
#define T_DETAIL_TITLE TEXT("Letzte Meldungen:")
#define T_DETAIL       TEXT("Raum 200;Anlage 1;12.12.2017;07:15:12;Messtisch 3|\
                             Raum 302;Anlage 3;14.12.2017;12:22:35;Messtisch 27")
#define T_SIGNATURE    TEXT("Johannes Brautzsch;\
  						     02.4V 0.15;\
                             Tel: 2906;\
                             j.brautzsch@fz-juelich.de")

#define ATTACHMENT_PATH TEXT("..\\SampleContent\\Test-Anhang.csv")
#define SUBJECT         TEXT("Test Email")
#define RECIPIENTS      TEXT("j.brautzsch@fz-juelich.de;\
                              johannes.brautzsch@gmail.com")

int _tmain(int argc, _TCHAR *argv[])
{
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

/*****************************************************************************/
/*                               Versionsinfos                               */
/*****************************************************************************/
  // Version des Testprogramms hier
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
/*                   SENDEN PER UMSCHLIESSENDEN FUNKTIONEN                   */
/*****************************************************************************/
#if defined(BENUTZE_DIREKTE_FUNKTIONEN)
  // ---- Server Initialisieren -----
  MailLibInit(SMPTSERVER,
              SMPTPORT,
              SMPTSECURITY,
              SENDER,
              SENDERNAME,
              MAILERNAME);
  MailLibSetAuthentification(USER, PASS);
#ifdef ATTACHMENT_PATH
  // ---- Anhang -----
  MailLibAddAttachment(ATTACHMENT_PATH);
#endif
#ifdef SENDE_EINFACHE_HTML_EMAIL
  // Datei einlesen, Setzen & Senden
  bSuccess = MailLibSendMail(RECIPIENTS,
                             SUBJECT,
                             MailLibReadFromFile(CONTENT_FILE),
                             TRUE);
#endif
#ifdef SENDE_EMAIL_UEBER_HTML_TEMPLATE
  // Email mit HTML-Template
  // Setzen & Senden
  bSuccess = MailLibSendMailByHTMLTemplateFile(RECIPIENTS,
                                               SUBJECT,
                                               CONTENT_TEMPL_FILE,
                                               T_SUBTITLE,
                                               T_LIST_TITLE,
                                               T_LIST,
                                               T_DETAIL_TITLE,
                                               T_DETAIL,
                                               T_SIGNATURE);
#endif
#ifdef SENDE_TEXT_EMAIL
  // Setzen & Senden
  bSuccess = MailLibSendMail(RECIPIENTS, SUBJECT, CONTENT, TRUE);
#endif
#endif

/*****************************************************************************/
/*                      DIREKTES SETZEN DER MAIL-STRUKTUR                    */
/*****************************************************************************/
#if defined(SETZE_MAIL_STRUKTUR_SELBST)
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
#ifdef ATTACHMENT_PATH
  // Pfad der Datei
  MailLibVectorInit(&(grMail.AttachVec));
  MailLibVectorAdd(&(grMail.AttachVec), ATTACHMENT_PATH);
#endif
// ---- Setzen des Inhalts ----
#ifdef SENDE_EINFACHE_HTML_EMAIL
  // HTML-Inhalt
  grMail.szContent = MailLibReadFromFile(CONTENT_FILE);
  grMail.bHTML = true;
  // Senden
  bSuccess = SmtpSendMail(&grMail);
#endif
#ifdef SENDE_EMAIL_UEBER_HTML_TEMPLATE
  // Email mit HTML-Template
  grMail.szContent = PopulateTemplateByContent(CONTENT_TEMPL_FILE,
                                               T_SUBTITLE,
                                               T_LIST_TITLE,
                                               T_LIST,
                                               T_DETAIL_TITLE,
                                               T_DETAIL,
                                               T_SIGNATURE);
  grMail.bHTML = TRUE;
  // Senden
  bSuccess = SmtpSendMail(&grMail);
#endif
#ifdef SENDE_TEXT_EMAIL
  // Textinhalt
  grMail.szContent = CONTENT;
  // Senden
  bSuccess = SmtpSendMail(&grMail);
#endif
#endif

  std::cout << std::endl
            << "DLL-Test: SmtpSendMail war "
            << (bSuccess ? "" : "nicht ")
            << "erfolgreich"
            << std::endl;

/*****************************************************************************/
/*                         Teile String in C-Vector                          */
/*****************************************************************************/
#ifdef VEKTOR_TEST
  CVECTOR TestVector;
  MailLibVectorInit(&TestVector);
  char *TestString = TEXT("c.onsuela@fz-juelich.de,
                           m.holtkoetter@fz-juelich.de;
                           markus.holtkoetter@dialup.fh-aachen.de,
                           abc@example.com");
  char *TestDelimiter = TEXT(";,");
  StringToVector(&TestVector, TestString, TestDelimiter);

  std::cout << "\nDLL-Test: Teststring: \""
            << TestString
            << "\" wird aufgeteilt:" << std::endl;
  for (int i = 0; i < MailLibVectorCount(&TestVector);
       i++)
{ /* Ausgabe der Vektor-Elemente */
    std::cout << (char *)MailLibVectorGet(&TestVector, i) << std::endl;
  }
#endif

  printf("\nDLL-Test: finished, press any key...\n");
  getchar();

  return 0;
}
