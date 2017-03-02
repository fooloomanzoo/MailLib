#ifndef POP3_H_INCLUDED
#define POP3_H_INCLUDED

#include "../MailLib.h"

#define MAX_POP3_ERR_MSG_SIZE 199 /*Größe des Puffers für Fehleraussgaben*/
#define RESPONSE_POP3_TIME_OUT_MS 500L /*Anzahl der Millisekunden für Timeout*/

int Pop3_Receive_Mail(MailType* pMailData, int bDeleteMail, const TCHAR* szAppendPath, char* szErrMsg);
int Pop3_Free_Data(MailType* pMailData);

#endif
