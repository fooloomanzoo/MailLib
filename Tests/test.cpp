#include "Windows.h"
#include <stdlib.h>
#include <iostream>

#include "../MailDll.h"

int main(int argc, char* argv[])
{

	bool bSuccess;

	/*************************************************************/
	/*             DIREKTES SETZEN DER MAIL-STRUKTUR             */
	/*************************************************************/
	// Beginn der Initialierung der Mail-Parameter-Struktur
	// Struktur anlegen
	MailType grMail;

	// Empfänger
	TCHAR rgRecip[] = TEXT("j.brautzsch@fz-juelich.de");
	TCHAR* rgRecips[] = { rgRecip };
	grMail.RecipVec = { rgRecips };

	// Adresse der Kopienempfänger
	grMail.CcRecipVec = NULL;

	// Adresse der Blind-Kopienempfänger
	grMail.BccRecipVec = NULL;

	// Anhang
#ifdef APPEND_ATTACHMENT
	// Pfad der Datei
	TCHAR szTxtPath[] = TEXT("C:\\test.txt");
	TCHAR* rgAttachPaths[] = { szTxtPath };
	grMail.AttachVec = rgAttachPaths;
#else
	grMail.AttachVec = NULL;
#endif
	// Smtp-Server
	grMail.szSmtpServer = TEXT("mailrelay.fz-juelich.de");
	grMail.szSmtpServerPort = 587;
	grMail.szSmtpSecurityType = USE_TLS;
	grMail.szSmtpPriority = XPRIORITY_NORMAL;

	// Authentifizierung
	grMail.szSmtpUser = TEXT("j.brautzsch");
	grMail.szSmtpPass = TEXT("(jab123)");

	// Absender
	grMail.szSenderAddress = TEXT("j.brautzsch@fz-juelich.de");
	grMail.szSenderName = TEXT("Johannes Brautzsch");
	grMail.szReplyAddress = TEXT("j.brautzsch@fz-juelich.de");

	// Mailer
	grMail.szMailerName = TEXT("MailLibDll-Tester");

	// Inhalt und Betreff
	grMail.szSubject = TEXT("Subject-Betreff");
	grMail.szMail = TEXT("Dies ist ein Testtext\n\nTest Ende!");

	// Senden
	bSuccess = SmtpSendMail(&grMail);

	std::cout << std::endl << "DLL: SmtpSendMail war " << (bSuccess ? "" : "nicht ") << "erfolgreich" << std::endl;

	printf("\nDLL: finished, press any key...\n");
	getchar();

	return 0;
}

