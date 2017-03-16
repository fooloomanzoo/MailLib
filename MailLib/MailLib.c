/****************************************************************************/
// Funktion: void InitMailStruct(MailType* m)
//
// Zweck: Initialisiert einen "MailType"-Struct
//
/****************************************************************************/
#pragma once

#include "MailLib.h"

void InitMailStruct(MailType *m)
{
	if (m->_initialized == TRUE)
	{
		FreeMailStruct(m);
	}

	m->szSmtpServer = (TCHAR *)malloc(sizeof(TCHAR));
	m->szMailerName = (TCHAR *)malloc(sizeof(TCHAR));
	m->szSmtpUser = (TCHAR *)malloc(sizeof(TCHAR));
	m->szSmtpPass = (TCHAR *)malloc(sizeof(TCHAR));
	m->szSenderName = (TCHAR *)malloc(sizeof(TCHAR));
	m->szSenderAddress = (TCHAR *)malloc(sizeof(TCHAR));
	m->szReplyAddress = (TCHAR *)malloc(sizeof(TCHAR));
	m->szSubject = (TCHAR *)malloc(sizeof(TCHAR));
	m->szContent = (TCHAR *)malloc(sizeof(TCHAR));

	m->szSmtpServer[0] = '\0';
	m->szMailerName[0] = '\0';
	m->szSmtpUser[0] = '\0';
	m->szSmtpPass[0] = '\0';
	m->szSenderName[0] = '\0';
	m->szSenderAddress[0] = '\0';
	m->szReplyAddress[0] = '\0';
	m->szSubject[0] = '\0';
	m->szContent[0] = '\0';

	vector_init(&(m->RecipVec));
	vector_init(&(m->CcRecipVec));
	vector_init(&(m->BccRecipVec));
	vector_init(&(m->AttachVec));

	m->szSmtpServerPort = 25;
	m->iSmtpSecurityType = NO_SECURITY;
	m->iSmtpPriority = XPRIORITY_NORMAL;
	m->bHTML = FALSE;
	m->_initialized = TRUE;
}

/****************************************************************************/
// Funktion: void FreeMailStruct(MailType* m)
//
// Zweck: Deallokiert Variablen eines "MailType"-Structs
//
/****************************************************************************/

void FreeMailStruct(MailType *m)
{
	if (m->_initialized != TRUE)
	{
		return;
	}

	free(m->szSmtpServer);
	free(m->szMailerName);
	free(m->szSmtpUser);
	free(m->szSmtpPass);
	free(m->szSenderName);
	free(m->szSenderAddress);
	free(m->szReplyAddress);
	free(m->szSubject);
	free(m->szContent);

	m->szSmtpServer = NULL;
	m->szMailerName = NULL;
	m->szSmtpUser = NULL;
	m->szSmtpPass = NULL;
	m->szSenderName = NULL;
	m->szSenderAddress = NULL;
	m->szReplyAddress = NULL;
	m->szSubject = NULL;
	m->szContent = NULL;

	vector_free(&(m->AttachVec));
	vector_free(&(m->RecipVec));
	vector_free(&(m->BccRecipVec));
	vector_free(&(m->CcRecipVec));

	// um zu verhindern, dass mehrmals versucht wird, den Speicher freizugeben
	m->_initialized = FALSE;
}