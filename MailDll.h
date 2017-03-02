#ifndef MAIL_DLL_H_INCLUDED
#define MAIL_DLL_H_INCLUDED

#ifdef MAIL_DLL_EXPORTS
#define MAIL_DLL_EXPORT __declspec(dllexport) 
#else
#define MAIL_DLL_EXPORT __declspec(dllimport) 
#endif

#include "MailLib.h"

MAIL_DLL_EXPORT int MailLibDirInfoSize(void);
MAIL_DLL_EXPORT int MailLibDirInfo(char *szInfo);
MAIL_DLL_EXPORT int MailLibVerInfo(char *szInfo);
MAIL_DLL_EXPORT int MailLibInit(char* szSmtpServer, char* szSenderAddress, char* szSenderName, char* szMailerName);
MAIL_DLL_EXPORT int MailLibSendMail(char* szEmpfaengerAddress, char* szSubject, char* szMail);
MAIL_DLL_EXPORT int MailLibAddAttachement(char* szTextAttachment);
MAIL_DLL_EXPORT int MailLibAuthentifizierung(char* szPop3User, char* szPop3Passwort);
MAIL_DLL_EXPORT int MailLibCopyFile(char* szSrcFile, char* szDestFile);
MAIL_DLL_EXPORT int RecipSplit(char*** rgRecips, char* adresses);
MAIL_DLL_EXPORT int SecSeit1970(int nDay, int nMonth, int nYear, int nHour, int nMinute);

/*Funktion sendet spezifizierten Text (evtl. leeren String) an die angegebenen Empfänger*/
/*Zum Mailversand muss auf jeden Fall der Smtp-Server (szSmtpServer-Komponente),
die Adresse des Senders (szSenderAddress-Komponente), die Anzahl der Kopien (iRecip-, iCcRecip-, iBccRecip-Komponente)
und ggf. (>0) die entsprechenden Stringvektoren (RecipVec-, BccRecipVec-, CcRecipVec-Komponente) gesetzt werden,
der Rest kann Null sein*/

MAIL_DLL_EXPORT bool SmtpSendMail(MailType* m);
typedef BOOL(*PF_SMTP_SEND_MAIL_T)(MailType* m);

MAIL_DLL_EXPORT BOOL MapiSendMail(MailType* m);
typedef BOOL(*PF_MAPI_SEND_MAIL_T)(MailType* m);

/*Funktion kann sukzessive aufgerufen werden, um alle eingegangen Mails (solange Rückgabewert gleich drei) abzurufen*/
/*Zum E-Mailabruf muss nur die szPop3Server-, die szPop3User- und die szPop3Pass-Komponente der MailType-Struktur gesetzt werden*/
MAIL_DLL_EXPORT INT Pop3ReceiveMail(MailType* pMailData, BOOL bDeleteMail, const TCHAR* szAppendPath);
typedef INT(*PF_POP3_RECEIVE_MAIL_T)(MailType* pMailData, BOOL bDeleteMail, const TCHAR* szAppendPath);
MAIL_DLL_EXPORT BOOL Pop3FreeData(MailType* pMailData);
typedef BOOL(*PF_POP3_FREE_DATA_T)(MailType* pMailData);

#endif
