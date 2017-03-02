#ifndef MAILLIB_H_INCLUDED
#define MAILLIB_H_INCLUDED

#ifndef _WIN32
#undef TCHAR
#define TCHAR char
#define TEXT(__TEXT_ARG__) (__TEXT_ARG__)
#define lstrlen(__LSTRLEN_ARG__) strlen(__LSTRLEN_ARG__)
#endif

#ifndef SMTP_SECURITY_DEFINTION
#define SMTP_SECURITY_DEFINTION
// TLS/SSL extension
enum SMTP_SECURITY_TYPE
{
	NO_SECURITY,
	USE_TLS,
	USE_SSL,
	DO_NOT_SET
};
#endif

#ifndef SMTP_PRIORITY_DEFINTION
#define SMTP_PRIORITY_DEFINTION
enum CSmptXPriority
{
	XPRIORITY_HIGH = 2,
	XPRIORITY_NORMAL = 3,
	XPRIORITY_LOW = 4
};
#endif

// Funktionspräfix für an Testpoint und LabView zu exportierende C-Funktionen, kein CPP !!!!
// Testpoint kann nur DLLs laden die kürzer als 8 Zeichen sind !!!
//#define DLL_FUNC extern "C" __declspec(dllexport)

typedef struct	{
					int iRecip; /*Anzahl der (direkten) Empfänger <->RecipVec*/
					int iCcRecip; /*Anzahl der (öffentlichen) Kopienempfänger <->CcRecipVec*/
					int iBccRecip; /*Anzahl der (versteckten) Kopienempfänger <->BccRecipVec*/
					int iAttach; /*Anzahl der anzuhängenden Textdateien <->AttachVec*/
					
					TCHAR	**RecipVec; /*Vektor von Strings mit Empfängeraddressen*/
					TCHAR	**CcRecipVec; /*Vektor von Strings mit (öffentlichen) Kopienempfängeraddressen*/
					TCHAR	**BccRecipVec; /*Vektor von Strings mit (versteckten) Kopienempfängeraddressen*/
					TCHAR	**AttachVec; /*Vektor von Strings mit vollständigen Pfaden zu den anzuhängenden Textdateien*/

					TCHAR	*szSenderName; /*Name des Senders (kann Null sein, nur für SmtpSend, Pop3ReceiveMail relevant)*/
					TCHAR	*szSenderAddress; /*(Email-)Adresse des Senders (nur für SmtpSend relevant)*/
					TCHAR	*szReplyAddress; /*Adresse für Rückantworten (kann Null sein)*/

					TCHAR	*szSmtpServer; /*Name des Smtp-Servers (nur für SmtpSend relevant)*/
					int	    szSmtpServerPort; /*Port des Smtp-Servers (Standards fürs Senden: Keine Verschlüsselung --> 25, TLS/SSL --> 465 oder 587) */
					enum SMTP_SECURITY_TYPE szSmtpSecurityType; /*Security Type des Smtp-Servers (NO_SECURITY --> 0, USE_TLS --> 1, USE_SSL --> 2, DO_NOT_SET --> 3) */
					enum CSmptXPriority szSmtpPriority; /*Sendepriotität (XPRIORITY_HIGH --> 2, XPRIORITY_NORMAL --> 3, XPRIORITY_LOW --> 4) */
					TCHAR   *szSmtpUser; /*Username des Smtp-Accounts (falls null, wird szPop3User benutzt)*/
					TCHAR   *szSmtpPass; /*Passwort des Smtp-Accounts (falls null, wird szPop3Pass benutzt)*/

					TCHAR   *szPop3Server;  /*Name oder IP-Adresse des Pop3-Servers (nur für Pop3ReceiveMail relevant)*/
					TCHAR   *szPop3User; /*Username des Pop3-Accounts*/
					TCHAR   *szPop3Pass; /*Passwort des Pop3-Accounts*/

					TCHAR	*szMailerName; /*Name des Mailingsprogramms (kann Null sein, nur für SmtpSend, Pop3ReceiveMail relevant)*/
					TCHAR	*szSubject; /*Betreff der Email (kann Null sein)*/
					TCHAR	*szMail; /*(Text-)Inhalt der Email (kann Null sein)*/
				} MailType;

#endif
