#pragma once
#ifndef MAIL_DLL_H_INCLUDED
  #define MAIL_DLL_H_INCLUDED

#ifdef MAIL_DLL_EXPORTS
  #define MAIL_DLL_EXPORT __declspec(dllexport)
#else
  #define MAIL_DLL_EXPORT __declspec(dllimport)
#endif

// C-Array Groeßen
#ifndef RETURNSIZE
  #define RETURNSIZE 255
#endif
#ifndef STRINGSIZE
  #define STRINGSIZE 255
#endif
#ifndef MAX_PATH
  #define MAX_PATH 260
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "MailLib.h"
#include "Utils/vector.h"

// ------------------------- Versionsinformationen ------------------------- //
MAIL_DLL_EXPORT int MailLibDirInfoSize(void);
MAIL_DLL_EXPORT int MailLibDirInfo(char *szInfo);
MAIL_DLL_EXPORT int VerInfo(char *szInfo);

// ------------------------ direktes Setzen und Senden --------------------- //
MAIL_DLL_EXPORT int MailLibInit(char *szSmtpServer,
                                int szSmtpPort,
                                int szSmtpSecurityType,
                                char *szSenderAddress,
                                char *szSenderName,
                                char *szMailerName);
MAIL_DLL_EXPORT void MailLibSetAuthentification(char *szUsername,
                                                char *szUserPassword);

MAIL_DLL_EXPORT int  MailLibAddAttachment(char *szFilePath);
MAIL_DLL_EXPORT void MailLibClearAttachment();

MAIL_DLL_EXPORT void MailLibAddRecipients(char *szRecipients);
MAIL_DLL_EXPORT void MailLibAddCcRecipients(char *szCcRecipients);
MAIL_DLL_EXPORT void MailLibAddBccRecipients(char *szBccRecipients);
MAIL_DLL_EXPORT void MailLibClearAllRecipients();

MAIL_DLL_EXPORT int MailLibSendMail(char *szRecip,
                                    char *szSubject,
                                    char *szMail,
                                    BOOL bHTML);
MAIL_DLL_EXPORT int MailLibSendMailByHTMLTemplateFile(char *szRecip,
                                                      char *szSubject,
                                                      char *szTemplateFilePath,
                                                      char *szSubtitle,
                                                      char *szListTitle,
                                                      char *szList,
                                                      char *szDetailTitle,
                                                      char *szDetailTable,
                                                      char *szSignaturList);


// --------------------- indirektes Setzten und Senden --------------------- //
MAIL_DLL_EXPORT BOOL SmtpSendMail(MailType *m);
MAIL_DLL_EXPORT char *
PopulateTemplateByContent(char *szTemplateFilePath,
                          char *szSubtitle,
                          char *szListTitle,
                          char *szList,
                          char *szDetailTitle,
                          char *szDetailTable,
                          char *szSignaturList);

// --------------------------- Hilfsfunktionen ----------------------------- //
MAIL_DLL_EXPORT void StringToVector(CVECTOR *pVec,
                                    char *szString,
                                    char *szDelimiter);
MAIL_DLL_EXPORT int SecSeit1970(int nDay,
                                int nMonth,
                                int nYear,
                                int nHour,
                                int nMinute);
MAIL_DLL_EXPORT char *MailLibReadFromFile(char *szFilePath);
MAIL_DLL_EXPORT int MailLibCopyFile(char *szSrcFile,
                                    char *szDestFile);

MAIL_DLL_EXPORT void MailLibVectorInit(CVECTOR *v);
MAIL_DLL_EXPORT int MailLibVectorCount(CVECTOR *v);
MAIL_DLL_EXPORT void MailLibVectorAdd(CVECTOR *v, void *e);
MAIL_DLL_EXPORT void MailLibVectorSet(CVECTOR *v, int i, void *e);
MAIL_DLL_EXPORT void *MailLibVectorGet(CVECTOR *v, int i);
MAIL_DLL_EXPORT void *MailLibVectorDelete(CVECTOR *v, int i);
MAIL_DLL_EXPORT void MailLibVectorFree(CVECTOR *v);

// --------------------- nicht exportierte Funktionen ---------------------- //
void ReplaceInString(std::string &subject,
               const std::string &search,
               const std::string &replace);
std::string VectorToHTML(CVECTOR *Vec,
                         const std::string &szStartTag,
                         const std::string &szEndTag);
char *CopyAndResize(char *dest, const char *orig);
bool FileExists(const char *szFilePath);

#ifdef __cplusplus
}
#endif

#endif
