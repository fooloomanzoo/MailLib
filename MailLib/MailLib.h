#pragma once
#ifndef MAILLIB_H_INCLUDED
  #define MAILLIB_H_INCLUDED

//#define MAIL_DLL_VERBOSE

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WIN32
  #undef TCHAR
  #define TCHAR TCHAR
  #define TEXT(__TEXT_ARG__) (__TEXT_ARG__)
  #define lstrlen(__LSTRLEN_ARG__) strlen(__LSTRLEN_ARG__)
#endif

#include "Utils/vector.h"
#include "windows.h"

// Funktionspraefix fuer an Testpoint und LabView zu exportierende C-Funktionen,
// kein CPP! Testpoint kann nur DLLs laden die kuerzer als 8 Zeichen sind!

#ifndef SMTP_SECURITY_DEFINTION
  #define SMTP_SECURITY_DEFINTION
  // TLS/SSL extension
  enum SmtpSecurityType {
    NO_SECURITY,
    USE_TLS,
    USE_SSL,
    DO_NOT_SET
  };
#endif

#ifndef SMTP_PRIORITY_DEFINTION
  #define SMTP_PRIORITY_DEFINTION
  enum SmptPriority {
    XPRIORITY_VERY_HIGH,
    XPRIORITY_HIGH,
    XPRIORITY_NORMAL,
    XPRIORITY_LOW
  };
#endif

#ifndef TEMPLATE_TOKENS
  #define TEMPLATE_TOKENS
  #define TEMPLATE_SUBTITLE_LOOKUP TEXT("<!--{{SUBTITLE}}-->")
  #define TEMPLATE_LIST_TITLE_LOOKUP TEXT("<!--{{LIST_TITLE}}-->")
  #define TEMPLATE_LIST_LOOKUP TEXT("<!--{{LIST}}-->")
  #define TEMPLATE_DETAIL_TITLE_LOOKUP TEXT("<!--{{DETAIL_TITLE}}-->")
  #define TEMPLATE_DETAIL_LOOKUP TEXT("<!--{{DETAIL}}-->")
  #define TEMPLATE_SIGNATURE_LOOKUP TEXT("<!--{{SIGNATURE}}-->")
  #define TEMPLATE_LIST_DELIMITER TEXT(";")
  #define TEMPLATE_TABLE_ROW_DELIMITER TEXT("|")
  #define TEMPLATE_TABLE_COLUMN_DELIMITER TEXT(";")
  #define TEMPLATE_SIGNATUR_DELIMITER TEXT(";")
#endif

typedef struct MAIL_TYPE {
  TCHAR *szSmtpServer;  /* Name des Smtp-Servers */
  int  szSmtpServerPort;/* Port des Smtp-Servers (Standards fuers Senden: Keine Verschluesselung --> 25, TLS/SSL --> 465 oder 587)*/
  enum SmtpSecurityType iSmtpSecurityType;
                        /* Security Type des Smtp-Servers (NO_SECURITY --> 0, USE_TLS --> 1, USE_SSL --> 2, DO_NOT_SET --> 3) */
  enum SmptPriority iSmtpPriority;
                        /* Sendeprioritaet (XPRIORITY_VERY_HIGH --> 1, XPRIORITY_HIGH --> 2, XPRIORITY_NORMAL --> 3, XPRIORITY_LOW --> 4) */
  TCHAR *szMailerName;  /* Name des Mailingsprogramms */
  TCHAR *szSmtpUser;    /* Username des Smtp-Accounts */
  TCHAR *szSmtpPass;    /* Passwort des Smtp-Accounts */

  TCHAR *szSenderName;    /* Name des Absenders  */
  TCHAR *szSenderAddress; /* Email-Adresse des Absenders */
  TCHAR *szReplyAddress;  /* Adresse fuer Rueckantworten  */
  TCHAR *szSubject;       /* Betreff der Email */
  TCHAR *szContent;       /* Text-Inhalt der Email */
  BOOL   bHTML;           /* Text-Inhalt ist im HTML-Format */

  CVECTOR RecipVec;    /* Vektor von Strings mit Empfaengeraddressen */
  CVECTOR CcRecipVec;  /* Vektor von Strings mit (oeffentlichen) KopienEmpfaengeraddressen */
  CVECTOR BccRecipVec; /* Vektor von Strings mit (versteckten) KopienEmpfaengeraddressen */
  CVECTOR AttachVec;   /* Vektor von Strings mit vollstaendigen Pfaden zu den anzuhaengenden Textdateien */

  BOOL _initialized;   /* Zeigt an, ob die Struktur initialisiert wurde (fuer Funktionen "InitMailStruct" und "FreeMailStruct") */

} MailType;

void InitMailStruct(MailType *m); /* Initialisiert MailType-Struct */
void FreeMailStruct(MailType *m); /* Deallokiert Variablen v. MailType-Struct*/

#ifdef __cplusplus
}
#endif

#endif
