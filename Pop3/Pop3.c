/***********************************************************************************/
/* Autor:  Benjamin Bruns                  Telefon: 3082                           */
/* E-Mail: b.bruns@fz-juelich.de           Zeitraum: 2006                          */
/*                                                                                 */
/***********************************************************************************/
/* Pop3-Modul: Empfangen von Emails über das POP3-Protokoll vom einem Mailserver   */
/*                                                                                 */
/***********************************************************************************/
/* Programmiersprache: C        *                                                  */
/*                              * Rechner:  PC INTEL PENTIUM 4 3 GHz               */
/*                              *                                                  */
/* Compiler: Visual C++         *                                                  */
/*      Version 2005            * Betriebssystem: Windows XP SP2                   */
/*                                                                                 */
/***********************************************************************************/
/* Abhängigkeiten:    Util-Modul                                                   */
/*                                                                                 */
/***********************************************************************************/

#ifdef _WIN32
#include <windows.h>
#include <wininet.h>
#include <TCHAR.h>
#include <shlwapi.h>

#else
#define MAKELONG(a, b) \ 
    ((long) (((short) (a)) | ((long) ((short) (b))) << 16))
#define StrStr strstr
#define StrStrI generic_strstri
#define StrChr strchr
#define _tcsnicmp strncmpi
#define lstrlen strlen
#define lstrcpy strcpy
#define lstrcmp strcmp
#define lstrcat strcat
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../Utils/Util.h"
#include "../Utils/GenericFuncs.h"
#include "Pop3.h"
#include "../Utils/Base64/Base64_0.h"
#include "../Utils/Md5/HmacMd5.h"

/*Globale Struktur, die gewährleistet, dass mit der Pop3-Empfangsfunktion sukzessive die richtigen (ungelesen) Emails abgerufen werden*/
typedef struct { TCHAR* szServer; TCHAR* szUser; unsigned int nAdressNmb; MailType** pAdressVec; unsigned int* rgMailNmb; } GlobalType;
static GlobalType* Global = NULL;
static unsigned int nMailBoxesNmb = 0;

#define MAX_REPLY_LINE 999 /*nach RFC 1939 512B*/
#define MAX_COMMAND_LINE (4+2*(40)+2) /*nach RFC 1939*/
#define POP3_PORT 110

static bool GetPop3Reply(generic_sd_t sd, char* szReply, size_t nStrSize);
static int Pop3CleanUp(generic_sd_t sd, MailType* pMailData, char* szReply, size_t nReplySize, int iRes);
static void ParseEmail(MailType* pMailData, const TCHAR* szAppendPath);

static char* StrCatDynA(char** szDest, size_t* StrSize, const char* szAppend);

/************************************************************************************/
/* Name der Funktion: Pop3_Receive_Mail                                             */
/*                                                                                  */
/* Aufgabe: Empfängt sukzessive neues Emails mit dem POP3-Protokoll                 */
/*                                                                                  */
/* Eingabeparameter: pMailData (beinhaltet alle notw. Inform. zum E-Mailempfang)    */
/*                   bDeleteMail (entscheidet übers serverseitige Löschen der Email)*/
/*                   szAppendPath (Pfad zur Ablage der angehängten Dateien)         */
/*                                (bei NULL werden keine Anhänge abgespeichert)     */
/*                   szErrMsg(enthält bei szDebug!=NULL erläuternden Fehlertext)    */
/*                           (SMTP_DEBUG def.: zus. Gebrauch zur Kommunikationsanz.)*/
/*                                                                                  */
/* Rückgabewerte:    0->Emailempfang fehlgeschlagen                                 */
/*                   1->keine Emails abrufbar                                       */
/*                   2->Emailempfang erfolgreich, keine weiteren Emails verfügbar   */
/*                   3->Emailempfang erfolgreich, weitere Emails verfügbar          */
/*                                                                                  */
/* Verwendete Funktionen: GetPop3Reply, Pop3CleanUp                                 */
/*                                                                                  */
/************************************************************************************/
int Pop3_Receive_Mail(MailType* pMailData, int bDeleteMail, const TCHAR* szAppendPath, char* szErrMsg)
{ /*Kommunikation erfolgt nach dem Frage-Antwort-Prinzip, nach RFC 1939*/
	generic_sd_t sd=generic_invalid_socket;

	char szIPAddr[16] = "", szChar[MAX_COMMAND_LINE+1] = "";
	char *szReply = NULL, *ptr = NULL, *LineBeg;
	char szCmd[MAX_COMMAND_LINE+1] = "";
	int i, l, rc;
	unsigned int nMessages=0, nMessageSize, nDummy=0, nGlobalIndex, nAddressIndex;
	bool bSucc = false, bRepeated = false;
	TCHAR *pOldBuf = NULL;
	size_t nReplySize = MAX_REPLY_LINE+1;
	bool bMd5, bEnc;
	unsigned char *szMd5Key, rgMd5Digest[16];
	char *szAuth, *szAuthB64, szMd5Digest[33];
	size_t nAuthSize, nLen;

	if( pMailData==NULL )
		return 0;
	if( pMailData->szPop3Server==NULL || pMailData->szPop3User==NULL || pMailData->szPop3Pass==NULL )
		return 0;

	nReplySize = MAX_REPLY_LINE+1;
	szReply = (char*) generic_malloc(nReplySize*sizeof(char));
	if( szReply==NULL )
		return false;

	/*Beginn der Initialisierung der WinSock-Schnittstelle*/
	if( !generic_socket_init() ) /*mindestens WINSOCK v2.0*/
		return 0;

	if( CheckIp(pMailData->szPop3Server) )
	{
		Tchar2Ansi(pMailData->szPop3Server,szIPAddr,sizeof(szIPAddr));
	}
	else
	{
		Tchar2Ansi(pMailData->szPop3Server,szChar,sizeof(szChar));
		if( !generic_gethostbyname(szChar,szIPAddr) )
		{
			generic_snprintf(szErrMsg,MAX_POP3_ERR_MSG_SIZE,"Fehler beim Ermitteln der IP-Adresse des Pop3-Servers!");
			return Pop3CleanUp(generic_invalid_socket,NULL,szReply,nReplySize,0);
		}
	}

	sd = generic_stream_socket_create(); /*verbindungsorientiertes TCP*/
	if( sd==generic_invalid_socket )
	{
		generic_snprintf(szErrMsg,MAX_POP3_ERR_MSG_SIZE,"Fehler bei Socket-Erzeugung!");
		return Pop3CleanUp(generic_invalid_socket,NULL,szReply,nReplySize,0);
	}

	nAddressIndex = 0;
	for(nGlobalIndex=0; nGlobalIndex<nMailBoxesNmb; ++nGlobalIndex)
		if( lstrcmp(Global[nGlobalIndex].szServer,pMailData->szPop3Server)==0 &&
			lstrcmp(Global[nGlobalIndex].szUser,pMailData->szPop3User)==0 )
			break;
	if( nGlobalIndex==nMailBoxesNmb ) /*keinen Eintrag für aktuelle Mailbox auf spez. Mailserver gefunden->Erzeuge Eintrag*/
	{
		GlobalType* ptr = NULL;
		if( Global )
			ptr = (GlobalType*) generic_realloc(ptr,(nMailBoxesNmb+1)*sizeof(GlobalType));
		else
			ptr = (GlobalType*) generic_malloc(sizeof(GlobalType));

		if( ptr )
		{
			ptr->szServer = (TCHAR*) generic_malloc((lstrlen(pMailData->szPop3Server)+1)*sizeof(TCHAR));
			ptr->szUser = (TCHAR*) generic_malloc((lstrlen(pMailData->szPop3User)+1)*sizeof(TCHAR));
			ptr->nAdressNmb = 1;
			ptr->pAdressVec = (MailType**) generic_malloc(ptr->nAdressNmb*sizeof(MailType*));
			ptr->rgMailNmb = (unsigned int*) generic_malloc(ptr->nAdressNmb*sizeof(unsigned int));
		}
		
		if( ptr==NULL || ptr->szServer==NULL || ptr->szUser==NULL || ptr->pAdressVec==NULL || ptr->rgMailNmb==NULL )
		{
			/*Gebe hier letzte Komponenten des globalen Instanzvektors frei, weil Instanzzähler nicht erhöht wurde*/
			if( ptr && ptr->szServer )
				generic_free(ptr->szServer);
			if( ptr && ptr->szUser )
				generic_free(ptr->szUser);
			if( ptr && ptr->pAdressVec )
				generic_free(ptr->pAdressVec);
			if( ptr && ptr->rgMailNmb )
				generic_free(ptr->rgMailNmb);
			generic_snprintf(szErrMsg,MAX_POP3_ERR_MSG_SIZE,"Unerwarteter Initialisierungsfehler!");
			return Pop3CleanUp(generic_invalid_socket,NULL,szReply,nReplySize,0);
		}

		lstrcpy(ptr->szServer,pMailData->szPop3Server);
		lstrcpy(ptr->szUser,pMailData->szPop3User);
		ptr->pAdressVec[nAddressIndex] = pMailData;
		ptr->rgMailNmb[nAddressIndex] = 1;
		Global = ptr;
		nGlobalIndex = nMailBoxesNmb;
		++nMailBoxesNmb;
	}
	else /*Eintrag für spezifizierte Mailbox gefunden*/
	{
		MailType** pMailRef = NULL;
		unsigned int* pMailNmb = NULL;

		for(nAddressIndex=0; nAddressIndex<Global[nGlobalIndex].nAdressNmb; ++nAddressIndex)
			if( Global[nGlobalIndex].pAdressVec[nAddressIndex]==pMailData )
				break;
		
		if( nAddressIndex==Global[nGlobalIndex].nAdressNmb )
		{ /*eigene MailType-Struktur für bestehende Mailbox-Instanz->füge Strukturadresse als Instanzverweis hinzu*/
			pMailRef = (MailType**) generic_realloc(Global[nGlobalIndex].pAdressVec,(nAddressIndex+1)*sizeof(MailType*));
			pMailNmb = (unsigned int*) generic_malloc((nAddressIndex+1)*sizeof(unsigned int));

			if( pMailRef==NULL || pMailNmb==NULL )
			{
				/*Gebe hier nicht ggf. allokierten Speicher für Vektoren frei, da zu allokierter freigebener Speicher kein Problem ist*/
				if( pMailRef )
					Global[nGlobalIndex].pAdressVec = pMailRef;
				if( pMailNmb )
					Global[nGlobalIndex].rgMailNmb = pMailNmb;

				generic_snprintf(szErrMsg,MAX_POP3_ERR_MSG_SIZE,"Unerwarteter Initialisierungsfehler!");
				return Pop3CleanUp(generic_invalid_socket,NULL,szReply,nReplySize,0);
			}

			pMailRef[nAddressIndex] = pMailData;
			pMailNmb[nAddressIndex] = 1;
			Global[nGlobalIndex].pAdressVec = pMailRef;
			Global[nGlobalIndex].rgMailNmb = pMailNmb;
			++Global[nGlobalIndex].nAdressNmb;
		}
		else
		{ /*Adresse der Mailbox-Dateninstanz gefunden -> erneuter Mailabruf*/
			bRepeated = true;

			if( pMailData->szMail==NULL )
			{
				generic_snprintf(szErrMsg,MAX_POP3_ERR_MSG_SIZE,"Unerwarteter Initialisierungsfehler!");
				return Pop3CleanUp(generic_invalid_socket,NULL,szReply,nReplySize,0);
			}
		}
	}

	if( !bRepeated )
	{ /*Falls erster Aufruf für Mailbox-Instanz, setzte Instanzdaten auf NULL, damit keine ungültige Reallokation bei Mail-Parsing*/
		pMailData->szMail = NULL;
		pMailData->RecipVec = NULL;
		pMailData->iRecip = 0;
		pMailData->CcRecipVec = NULL;
		pMailData->iCcRecip = 0;
		pMailData->szSenderName = NULL;
		pMailData->szSenderAddress = NULL;
		pMailData->szReplyAddress = NULL;
		pMailData->szMailerName = NULL;
		pMailData->szSubject = NULL;
		pMailData->AttachVec = NULL;
		pMailData->iAttach = 0;
	}

	if( !generic_socket_connect(&sd,szIPAddr,POP3_PORT) )
	{
		generic_snprintf(szErrMsg,MAX_POP3_ERR_MSG_SIZE,"Fehler beim Verbinden mit Pop3-Server!");
		return Pop3CleanUp(generic_invalid_socket,pMailData,szReply,nReplySize,0);
	}

	strcpy(szReply,"");

	bSucc = GetPop3Reply(sd,szReply,nReplySize);
	bSucc = bSucc && generic_strncmpi(szReply,"+OK",3)==0;
	if( !bSucc  )
	{
		generic_snprintf(szErrMsg,MAX_POP3_ERR_MSG_SIZE,"Fehler beim Empfang der Begrüßungsnachricht!");
		return Pop3CleanUp(sd,pMailData,szReply,nReplySize,0);
	}

	do
	{
		bMd5 = pMailData->szPop3User!=NULL && pMailData->szPop3Pass!=NULL;
		if( !bMd5 ) /*Breche CRAM/MD5-Authentifizierung ab, wenn keine Login-Daten verfügbar*/
			break;

		/*Starte CRAM/MD5-Authentifizierungvorgang (Handshake)*/
		strncpy(szCmd,"AUTH CRAM-MD5\r\n",sizeof(szCmd));
		bSucc = generic_socket_send(sd,szCmd,strlen(szCmd));
		strcpy(szReply,"");
		bSucc = bSucc && GetPop3Reply(sd,szReply,nReplySize);
		bSucc = bSucc && strncmp(szReply,"+",1)==0;
		if( !bSucc )
		{
			bMd5 = false;
			break;
		}
		
		/*Extrahiere aus Aufforderung des Servers den Schlüssel zur Anwendung des HMAC-MD5-Verfahren auf das POP3-Passwort*/
		nLen = strlen(szReply);
		if( nLen>5 && szReply[1]==' ' )
			memmove(szReply,szReply+2,nLen-1);
		else
			strcpy(szReply,"");
		
		/*Rücktransformation des Base64-codierten MD5-Schlüssels*/
		szMd5Key = NULL;
		rc = B64Dec(&szMd5Key,szReply);
		bEnc = rc>=0; /*Rücktransformation erfolgreich, wenn transformierter Schlüssel mindestens 0 Zeichen lang ist*/

		/*Generieren der Antwort (exkl. CRLF) szAuth auf Serveranfrage aus POP3-Benutzername, Leerzeichen und gehashtem Passwort und anschließender Base64-Kodierung*/
		bEnc = bEnc && Tchar2Ansi(pMailData->szPop3User,szReply,nReplySize)!=NULL;
		szAuth = NULL; nAuthSize = 0;
		bEnc = bEnc && StrCatDynA(&szAuth,&nAuthSize,szReply)!=NULL;
		bEnc = bEnc && StrCatDynA(&szAuth,&nAuthSize," ")!=NULL;

		bEnc = bEnc && Tchar2Ansi(pMailData->szPop3Pass,szReply,nReplySize)!=NULL;
		bEnc = bEnc && hmac_md5(szMd5Key,rc,szReply,strlen(szReply),rgMd5Digest);
		for(i=0; i<16; ++i) /*Konvertierung des Byte-Streams in einen String in Hexadezimaldarstellung*/
			sprintf(szMd5Digest+2*i,"%02x",rgMd5Digest[i]);
		szMd5Digest[32] = '\0';

		bEnc = bEnc && StrCatDynA(&szAuth,&nAuthSize,szMd5Digest)!=NULL;
		szAuthB64 = NULL;
		bEnc = bEnc && B64Enc(&szAuthB64,szAuth,strlen(szAuth))>0;
		strcpy(szReply,"");
		bEnc = bEnc && StrCatDynA(&szReply,&nReplySize,szAuthB64);
		bEnc = bEnc && StrCatDynA(&szReply,&nReplySize,"\r\n");

		if( szMd5Key )
			free(szMd5Key);
		if( szAuthB64 )
			free(szAuthB64);
		if( szAuth )
			generic_free(szAuth);

		if( !bEnc )
		{
			generic_snprintf(szErrMsg,MAX_POP3_ERR_MSG_SIZE,"Anwendung des CRAM/MD5-Verfahrens fehlgeschlagen!");
			return Pop3CleanUp(sd,pMailData,szReply,nReplySize,0);
		}

		if( !generic_socket_send(sd,szReply,strlen(szReply)) )
		{
			generic_snprintf(szErrMsg,MAX_POP3_ERR_MSG_SIZE,"Fehler: Senden von <%s> fehlgeschlagen!",szReply);
			return Pop3CleanUp(sd,pMailData,szReply,nReplySize,0);
		}
		strcpy(szReply,"");
		bSucc = GetPop3Reply(sd,szReply,nReplySize);
		bSucc = bSucc && generic_strncmpi(szReply,"+OK",3)==0;
		if( !bSucc )
		{
			generic_snprintf(szErrMsg,MAX_POP3_ERR_MSG_SIZE,"Fehler: Server verweigert Zugang für %s:%s!",pMailData->szPop3User,pMailData->szPop3Pass);
			return Pop3CleanUp(sd,pMailData,szReply,nReplySize,0);
		}
	}
	while( 0 );

	if( !bMd5 )
	{
		Tchar2Ansi(pMailData->szPop3User,szChar,sizeof(szChar));
		generic_snprintf(szCmd,sizeof(szCmd),"USER %s\r\n",szChar);
		bSucc = generic_socket_send(sd,szCmd,strlen(szCmd));
		strcpy(szReply,"");
		bSucc = bSucc && GetPop3Reply(sd,szReply,nReplySize);
		bSucc = bSucc && generic_strncmpi(szReply,"+OK",3)==0;
		if( !bSucc )
		{
			generic_snprintf(szErrMsg,MAX_POP3_ERR_MSG_SIZE,"Fehler bei Spezifizierung der Mailbox!");
			return Pop3CleanUp(sd,pMailData,szReply,nReplySize,0);
		}

		Tchar2Ansi(pMailData->szPop3Pass,szChar,sizeof(szChar));
		generic_snprintf(szCmd,sizeof(szCmd),"PASS %s\r\n",szChar);
		bSucc = generic_socket_send(sd,szCmd,strlen(szCmd));
		strcpy(szReply,"");
		bSucc = bSucc && GetPop3Reply(sd,szReply,nReplySize);
		bSucc = bSucc && generic_strncmpi(szReply,"+OK",3)==0;
		if( !bSucc )
		{
			generic_snprintf(szErrMsg,MAX_POP3_ERR_MSG_SIZE,"Fehler bei der Authentifizierung!");
			return Pop3CleanUp(sd,pMailData,szReply,nReplySize,0);
		}
	}

	generic_snprintf(szCmd,sizeof(szCmd),"STAT\r\n");
	bSucc = generic_socket_send(sd,szCmd,strlen(szCmd));
	strcpy(szReply,"");
	bSucc = bSucc && GetPop3Reply(sd,szReply,nReplySize);
	bSucc = bSucc && generic_strncmpi(szReply,"+OK",3)==0;
	bSucc = bSucc && sscanf(szReply,"+OK %u %u",&nMessages,&nDummy)==2;
	if( !bSucc )
	{
		generic_snprintf(szErrMsg,MAX_POP3_ERR_MSG_SIZE,"Fehler bei der Ermittlung der Anzahl abrufbarer Emails!");
		return Pop3CleanUp(sd,pMailData,szReply,nReplySize,0);
	}
	if( nMessages<Global[nGlobalIndex].rgMailNmb[nAddressIndex] )
		return Pop3CleanUp(sd,pMailData,szReply,nReplySize,1); /*Keine neuen Emails zum Abrufen bereit*/

	generic_snprintf(szCmd,sizeof(szCmd),"LIST %u\r\n",Global[nGlobalIndex].rgMailNmb[nAddressIndex]);
	bSucc = generic_socket_send(sd,szCmd,strlen(szCmd));
	strcpy(szReply,"");
	bSucc = bSucc && GetPop3Reply(sd,szReply,nReplySize);
	bSucc = bSucc && generic_strncmpi(szReply,"+OK",3)==0;
	bSucc = bSucc && sscanf(szReply,"+OK %u %u",&nDummy,&nMessageSize)==2;
	if( !bSucc || nMessageSize<1 )
	{
		generic_snprintf(szErrMsg,MAX_POP3_ERR_MSG_SIZE,"Fehler bei der Ermittlung der Größe der abzurufenden Email!");
		return Pop3CleanUp(sd,pMailData,szReply,nReplySize,0);
	}

	if( bRepeated )
		pOldBuf = (TCHAR*) generic_realloc(pMailData->szMail,(nMessageSize+1)*sizeof(TCHAR));
	else
		pOldBuf = (TCHAR*) generic_malloc((nMessageSize+1)*sizeof(TCHAR));

	if( pOldBuf==NULL )
	{
		generic_snprintf(szErrMsg,MAX_POP3_ERR_MSG_SIZE,"Unerwarteter Fehler beim Emailabruf!");
		return Pop3CleanUp(sd,pMailData,szReply,nReplySize,0);
	}
	pMailData->szMail = pOldBuf;

	generic_snprintf(szCmd,sizeof(szCmd),"RETR %u\r\n",Global[nGlobalIndex].rgMailNmb[nAddressIndex]);
	bSucc = generic_socket_send(sd,szCmd,strlen(szCmd));
	strcpy(szReply,"");
	bSucc = bSucc && GetPop3Reply(sd,szReply,nReplySize);
	bSucc = bSucc && generic_strncmpi(szReply,"+OK",3)==0;

	l=0;
	bSucc = false;
	while( GetPop3Reply(sd,szReply,nReplySize) )
	{
		LineBeg = szReply;
		if( strncmp(LineBeg,".",1)==0 )
		{
			if( LineBeg[1]=='\0' )
			{ /*Ende der Mehrzeilenantwort nach RFC 1939*/
				bSucc = true;
				break;
			}
			else
				++LineBeg; /*Entfernen des ersten Oktets der Zeile (byte-stuffed line according to RFC 1939)*/
		}
			
		Ansi2Tchar(LineBeg,pMailData->szMail+l,nMessageSize-l);
		l+= strlen(szReply);
		lstrcpy(pMailData->szMail+l,TEXT("\n"));
		l+= 1;
	}
	
	if( !bSucc )
	{
		generic_snprintf(szErrMsg,MAX_POP3_ERR_MSG_SIZE,"Fehler bei Abrufen der Email!");
		return Pop3CleanUp(sd,pMailData,szReply,nReplySize,0);
	}
	
	rc = nMessages>Global[nGlobalIndex].rgMailNmb[nAddressIndex] ? 3 : 2;
	if( bDeleteMail )
	{
		generic_snprintf(szCmd,sizeof(szCmd),"DELE %u\r\n",Global[nGlobalIndex].rgMailNmb[nAddressIndex]);
		bSucc = generic_socket_send(sd,szCmd,strlen(szCmd));
		strcpy(szReply,"");
		bSucc = bSucc && GetPop3Reply(sd,szReply,nReplySize);
		bSucc = bSucc && generic_strncmpi(szReply,"+OK",3)==0;
		if( !bSucc )
		{
			generic_snprintf(szErrMsg,MAX_POP3_ERR_MSG_SIZE,"Fehler bei Löschen der Email!");
			return Pop3CleanUp(sd,pMailData,szReply,nReplySize,0);
		}
	}
	else
	{
		/*Erhöhe die Nummer der als nächstes einzulesenden Email*/
		++Global[nGlobalIndex].rgMailNmb[nAddressIndex];
	}

	Pop3CleanUp(sd,NULL,szReply,nReplySize,1); /*Gebe Puffer für Email nicht frei*/

	ParseEmail(pMailData,szAppendPath);
	return rc;
}

/************************************************************************************/
/* Name der Funktion: Pop3_Free_Data                                                */
/*                                                                                  */
/* Aufgabe: Gibt den von Pop3_Receive_Mail allokierten Speicher in der              */
/*          MailType-Struktur frei und ggf. in Mailbox-Instanzen (<->Global)       */
/*                                                                                  */
/* Eingabeparameter: pMailData (beinhaltet alle notw. Inform. zum E-Mailempfang)    */
/*                                                                                  */
/* Rückgabewerte:    Erfolg der Speicherfreigabe                                    */
/*                                                                                  */
/* Verwendete Funktionen: keine                                                     */
/*                                                                                  */
/* Anmerkung: Einmal aufzurufen, nachdem Emailsitzung für einen Account beendet ist */
/*                                                                                  */
/************************************************************************************/
int Pop3_Free_Data(MailType* pMailData)
{
	unsigned int i, j;

	if( pMailData==NULL )
		return false;

	/*gebe alle durch Pop3_Receive_Mail allokierten Speicher für Komponeten frei -> nicht den der szPop3Server-, szPop3User-, szPop3Pass-Komponenten*/
	if( pMailData->szMail )
	{
		generic_free(pMailData->szMail);
		pMailData->szMail = NULL;
	}
	if( pMailData->szSenderName )
	{
		generic_free(pMailData->szSenderName);
		pMailData->szSenderName = NULL;
	}
	if( pMailData->szSenderAddress )
	{
		generic_free(pMailData->szSenderAddress);
		pMailData->szSenderAddress = NULL;
	}
	if( pMailData->szReplyAddress )
	{
		generic_free(pMailData->szReplyAddress);
		pMailData->szReplyAddress = NULL;
	}
	if( pMailData->szSubject )
	{
		generic_free(pMailData->szSubject);
		pMailData->szSubject = NULL;
	}
	if( pMailData->szMailerName )
	{
		generic_free(pMailData->szMailerName);
		pMailData->szMailerName = NULL;
	}
	if( pMailData->AttachVec )
	{
		int i;
		for(i=0; i<pMailData->iAttach; ++i)
			if( pMailData->AttachVec[i] )
				generic_free(pMailData->AttachVec[i]);

		generic_free(pMailData->AttachVec);
		pMailData->AttachVec = NULL;
	}
	if( pMailData->CcRecipVec )
	{
		int i;
		for(i=0; i<pMailData->iCcRecip; ++i)
			if( pMailData->CcRecipVec[i] )
				generic_free(pMailData->CcRecipVec[i]);

		generic_free(pMailData->CcRecipVec);
		pMailData->CcRecipVec = NULL;
	}
	if( pMailData->RecipVec )
	{
		int i;
		for(i=0; i<pMailData->iRecip; ++i)
			if( pMailData->RecipVec[i] )
				generic_free(pMailData->RecipVec[i]);

		generic_free(pMailData->RecipVec);
		pMailData->RecipVec = NULL;
	}
	
	if( pMailData->szPop3Server==NULL || pMailData->szPop3User==NULL || nMailBoxesNmb==0 )
		return false;

	for(i=0; i<nMailBoxesNmb; ++i)
		if( lstrcmp(Global[i].szServer,pMailData->szPop3Server)==0 &&
			lstrcmp(Global[i].szUser,pMailData->szPop3User)==0 )
			break;
	if( i==nMailBoxesNmb )
		return false;
	for(j=0; j<Global[i].nAdressNmb; ++j)
		if( Global[i].pAdressVec[j]==pMailData )
			break;
	if( j==Global[i].nAdressNmb )
		return false;

	if( --Global[i].nAdressNmb == 0 )
	{
		if( Global[i].pAdressVec )
		{
			generic_free(Global[i].pAdressVec);
			Global[i].pAdressVec = NULL;
		}
		if( Global[i].rgMailNmb )
		{
			generic_free(Global[i].rgMailNmb);
			Global[i].rgMailNmb = NULL;
		}
		if( Global[i].szServer )
		{
			generic_free(Global[i].szServer);
			Global[i].szServer = NULL;
		}
		if( Global[i].szUser )
		{
			generic_free(Global[i].szUser);
			Global[i].szUser = NULL;
		}
		--nMailBoxesNmb;
	}

	if( nMailBoxesNmb==0 )
	{
		generic_free(Global);
		Global = NULL;
	}

	return true;
}

static int Pop3CleanUp(generic_sd_t sd, MailType* pMailData, char* szReply, size_t nReplySize, int iRes)
{ /*Aufräumarten nach (nicht) erfolgreichem Aufruf von Pop3_Receive_Mail*/
	char szCmd[] = "QUIT\r\n";
	
	if( sd!=generic_invalid_socket )
	{
		if( szReply )
			if( generic_socket_send(sd,szCmd,strlen(szCmd)) )
				GetPop3Reply(sd,szReply,nReplySize);
		generic_socket_close(sd);
		sd=generic_invalid_socket;
	}

	Pop3_Free_Data(pMailData);

	if( szReply )
	{
		generic_free(szReply);
		szReply = NULL;
	}

	generic_socket_finish();
	GetPop3Reply(sd,NULL,0); /*Freigabe des funktionslokal allokierten Speichers*/
	return iRes;
}

static bool GetPop3Reply(generic_sd_t sd, char* szReply, size_t nStrSize)
{ /*Empfängt Pop3-Serverantwort und liefert mindestens eine Zeile der Serverantwort zurück*/
    static unsigned char* rgRcvBuf = NULL;
	static size_t nRcvSize = 0;
	static int nRcvIdx = 0;
	static char szMsgDelim[] = "\r\n";

	size_t nLen, nBytesRead;
	unsigned char* pRcv;
	char* pDelim;
	unsigned long nPastTime;
	unsigned long nTimeOut;

	if( sd==INVALID_SOCKET || szReply==NULL || nStrSize<2 )
	{
		if( rgRcvBuf )
			generic_free(rgRcvBuf);
		rgRcvBuf = NULL;
		nRcvSize = 0;
		nRcvIdx = 0;

		return true;
	}

	#ifdef RESPONSE_POP3_TIME_OUT_MS
	nTimeOut = RESPONSE_POP3_TIME_OUT_MS;
	#else
	nTimeOut = ULONG_MAX;
	#endif

	while( rgRcvBuf==NULL || (pDelim=strstr(rgRcvBuf,szMsgDelim))==NULL )
	{
		if( nRcvSize-nRcvIdx<MAX_REPLY_LINE )
		{
			pRcv = (unsigned char*) generic_realloc(rgRcvBuf,3*(nRcvSize+MAX_REPLY_LINE)/2*sizeof(unsigned char));
			if( pRcv==NULL )
				return false;
			rgRcvBuf = pRcv;
			nRcvSize = 3*(nRcvSize+MAX_REPLY_LINE)/2;
		}

		nBytesRead=generic_socket_recv(sd,rgRcvBuf+nRcvIdx,nRcvSize-nRcvIdx-1,szMsgDelim,nTimeOut,&nPastTime);
		if( nBytesRead==0 )
			return false;
	
		nTimeOut-= nPastTime;
		nRcvIdx+= nBytesRead;
		rgRcvBuf[nRcvIdx] = '\0';
	}
	
	*pDelim = '\0';
	strncpy(szReply,rgRcvBuf,nStrSize-1);
	szReply[nStrSize-1] = '\0';
	nLen = strlen(pDelim+2);
	nRcvIdx = nLen;
	memmove(rgRcvBuf,pDelim+2,nLen+1);

	return true;
}

static TCHAR* PtrCpy(TCHAR** szDest, TCHAR* pTagBeg, TCHAR* pTagEnd);

static int ReadMailArray(TCHAR* szLine, TCHAR cDelim, TCHAR*** pDestArray);
static bool ReadMailField(TCHAR* szLine, bool bBrackets, TCHAR** szDest1, TCHAR** szDest2);
static bool FindMailParts(TCHAR* szMailBody, const TCHAR* szBoundary, unsigned int* pncPlainSection, long** prgPlainSection, unsigned int* pncAttachSection, long** prgAttachSection, const TCHAR* szAttachPath, int* pncAttachPath, TCHAR*** prgAttachPath);

static void ParseEmail(MailType* pMailData, const TCHAR* szAppendPath)
{ /*Extrahiert aus Emailquelltext den Header und trägt die jeweiligen Informationen in MailType-Struktur ein*/
	TCHAR *pLineBeg, *pLineEnd, *pTagBeg, *szBoundary=NULL, szLineDelim[]=TEXT("\n"), c;
	bool bMime=false;
	int l;

	if( pMailData->szMail==NULL )
		return;

	/*Parse nur nach LF-Zeilenumbrüchen, da Email beim Empfang zwecks mehrerer aufeinanderfolgende Leerzeilen so konvertiert wurde*/
	for(pLineBeg=pMailData->szMail; (pLineEnd=StrStr(pLineBeg,szLineDelim))!=NULL; pLineBeg=pLineEnd+lstrlen(szLineDelim))
	{
		c = *pLineEnd;
		*pLineEnd = TEXT('\0');

		if( _tcsnicmp(pLineBeg,TEXT("From: "),l=lstrlen(TEXT("From: ")))==0 )
		{
			ReadMailField(pLineBeg+l,true,&(pMailData->szSenderAddress),&(pMailData->szSenderName));
		}
		else if( _tcsnicmp(pLineBeg,TEXT("Subject: "),l=lstrlen(TEXT("Subject: ")))==0 )
		{
			ReadMailField(pLineBeg+l,false,&(pMailData->szSubject),NULL);
		}
		else if( _tcsnicmp(pLineBeg,TEXT("X-Mailer: "),l=lstrlen(TEXT("X-Mailer: ")))==0 )
		{
			ReadMailField(pLineBeg+l,false,&(pMailData->szMailerName),NULL);
		}
		else if( _tcsnicmp(pLineBeg,TEXT("Reply-To: "),l=lstrlen(TEXT("Reply-To: ")))==0 )
		{
			ReadMailField(pLineBeg+l,true,&(pMailData->szReplyAddress),NULL);
		}
		else if( _tcsnicmp(pLineBeg,TEXT("To: "),l=lstrlen(TEXT("To: ")))==0 )
		{
			pMailData->iRecip = ReadMailArray(pLineBeg+l,TEXT(','),&(pMailData->RecipVec));
		}
		else if( _tcsnicmp(pLineBeg,TEXT("Cc: "),l=lstrlen(TEXT("Cc: ")))==0 )
		{
			pMailData->iCcRecip = ReadMailArray(pLineBeg+l,TEXT(','),&(pMailData->CcRecipVec));
		}
		else if( _tcsnicmp(pLineBeg,TEXT("MIME-version: "),l=lstrlen(TEXT("MIME-version: ")))==0 )
		{
			TCHAR* szMimeVersion=NULL;
			int nMajVer, nMinVer;

			ReadMailField(pLineBeg+l,false,&szMimeVersion,NULL);
			if( szMimeVersion==NULL )
				continue;
			bMime = _stscanf(szMimeVersion,TEXT("%d.%d"),&nMajVer,&nMinVer)==2;
			bMime = bMime && nMajVer>=1;
		}
		else if( _tcsnicmp(pLineBeg,TEXT("Content-Type: Multipart/Mixed;"),l=lstrlen(TEXT("Content-Type: Multipart/Mixed;")))==0 )
		{
			pTagBeg=StrStr(pLineBeg,TEXT("boundary="));
			if( pTagBeg==NULL )
				continue;
			ReadMailField(pTagBeg+lstrlen(TEXT("boundary=")),false,&szBoundary,NULL);
		}
		else
		{
			for(pTagBeg=pLineBeg; _istspace(*pTagBeg); ++pTagBeg);
			if( *pTagBeg==TEXT('\0') ) /*Zeile besteht nur aus Zwischenraumzeichen -> Ende des Mail-Headers erreicht*/
			{
				*pLineEnd = c;
				break; 
			}
		}

		*pLineEnd = c;
	}

	bMime = bMime && szBoundary && lstrlen(szBoundary)>0;
	if( bMime && pLineEnd ) /*mögliche MIME MultiPart-Mail (Boundary-String gefunden und Mail-Body vorhanden)*/
	{
		unsigned int ncPlainSection, ncAttachSection, nBeg, nEnd; 
		long *rgPlainSection, *rgAttachSection;
		unsigned int i;
		TCHAR c, *pTmp, *szMailContent, szPlainDelim[]=TEXT("--NextPlainText--");
		FILE* pIn;

		if( pMailData->AttachVec )
		{
			generic_free(pMailData->AttachVec);
			pMailData->AttachVec = NULL;
			pMailData->iAttach = 0;
		}
		
		if( FindMailParts(pLineEnd,szBoundary,&ncPlainSection,&rgPlainSection,&ncAttachSection,&rgAttachSection,szAppendPath,&(pMailData->iAttach),&(pMailData->AttachVec)) )
		{
			for(i=0; i<ncAttachSection; ++i) /*Schreibe Dateianhänge in den spez. Pfad mit ausgelesenem Namen*/
			{
				pIn = _tfopen(pMailData->AttachVec[i],TEXT("w"));
				if( pIn==NULL )
					continue;

				nBeg = LOWORD(rgAttachSection[i]);
				nEnd = HIWORD(rgAttachSection[i]);

				c = pLineEnd[nEnd];
				pLineEnd[nEnd] = TEXT('\0');
				_ftprintf(pIn,TEXT("%s"),pLineEnd+nBeg);

				fclose(pIn);
				pLineEnd[nEnd] = c;
			}

			/*Initialisierung auf leeren String für evtl. wiederholten Aufruf, somit kein unerwarteter Initialisierungsfehler*/
			szMailContent = (TCHAR*) generic_malloc(1*sizeof(TCHAR));
			i= szMailContent ? 0 : ncPlainSection;
			lstrcpy(szPlainDelim,TEXT(""));
			for(; i<ncPlainSection; ++i) /*Schreibe eigentliche Mailnachricht(en) in neuen Puffer und überschreibe damit szMail-Komponente*/
			{
				nBeg = LOWORD(rgPlainSection[i]);
				nEnd = HIWORD(rgPlainSection[i]);

				pTmp = (TCHAR*) generic_realloc(szMailContent,(lstrlen(szMailContent)+lstrlen(szPlainDelim)+nEnd-nBeg+1)*sizeof(TCHAR));
				if( pTmp==NULL )
					continue;
				szMailContent = pTmp;
				lstrcat(szMailContent,szPlainDelim);
				lstrcpy(szPlainDelim,TEXT("--NextPlainText--"));

				for(pTmp=szMailContent+lstrlen(szMailContent); nBeg<nEnd; ++nBeg,++pTmp)
					*pTmp = pLineEnd[nBeg];
				*pTmp = TEXT('\0');
			}

			generic_free(pMailData->szMail);
			pMailData->szMail = szMailContent;
		}
	}

	if( szBoundary )
		generic_free(szBoundary);
}

static bool FindMailParts(TCHAR* szMailBody, const TCHAR* szBoundary, unsigned int* pncPlainSection, long** prgPlainSection, unsigned int* pncAttachSection, long** prgAttachSection, const TCHAR* szAttachPath, int* pncAttachPath, TCHAR*** prgAttachPath)
{
	TCHAR c, *pTmp1, *pTmp2, *pTmp3, *pTmp4, **pTmpMtx, *szPartDelim = NULL, *pPartBeg=NULL, *pPartEnd=NULL;
	int ncDelim = 0, ncPathPrefix, i;
	bool bAttach, bSucc;
	long **prgTmpSection, *pTmpVec;
	unsigned int *pncTmpSection;

	*pncPlainSection = *pncAttachSection = 0;
	*prgPlainSection = (long*) generic_malloc(1*sizeof(long));
	*prgAttachSection = (long*) generic_malloc(1*sizeof(long));
	*pncAttachPath = 0;
	*prgAttachPath = (TCHAR**) generic_malloc(1*sizeof(TCHAR*));
	
	ncDelim = lstrlen(szBoundary)+3;
	ncPathPrefix = szAttachPath ? lstrlen(szAttachPath) : 0;
	szPartDelim = (TCHAR*) generic_malloc((ncDelim+1)*sizeof(TCHAR));
	if( szPartDelim==NULL || *prgPlainSection==NULL || *prgAttachSection==NULL )
	{
		if( *prgPlainSection )
			generic_free(*prgPlainSection);
		*prgPlainSection = NULL;
		if( *prgAttachSection )
			generic_free(*prgAttachSection);
		*prgAttachSection = NULL;
		if( szPartDelim )
			generic_free(szPartDelim);
		if( *prgAttachPath )
		{
			generic_free(*prgAttachPath);
			*prgAttachPath = NULL;
		}

		return false;
	}
	lstrcpy(szPartDelim,TEXT("\n"));
	lstrcat(szPartDelim,TEXT("--"));
	lstrcat(szPartDelim,szBoundary);

	pPartBeg=szMailBody;
	while( (pPartBeg=StrStr(pPartBeg,szPartDelim))!=NULL )  /*Search for Boundary-String must be case-sensitiv*/
	{
		pPartBeg+= ncDelim;
		pPartEnd=StrStr(pPartBeg,szPartDelim);
		if( pPartEnd==NULL )
			break;
		
		c = *pPartEnd;
		*pPartEnd = TEXT('\0');

		pTmp1 = StrStrI(pPartBeg,TEXT("Content-Type: text/plain"));
		if( pTmp1 ) /*unterstützten Mail-Abschnitt mit Klartext gefunden*/
		{
			pTmp1+= lstrlen(TEXT("Content-Type: text/plain"));
			pTmp2 = StrStrI(pTmp1,TEXT("Content-Disposition: attachment"));
			bAttach = pTmp2!=NULL;
			
			if( bAttach )
			{
				bSucc = false;
				pTmp1 = StrStrI(pTmp1,TEXT("filename="));
				
				if( pTmp1 )
				{
					pTmp1+= lstrlen(TEXT("filename="));
					pTmp3 = StrChr(pTmp1,TEXT('"'));
					pTmp4 = NULL;
					if( pTmp3 )
						pTmp4=StrChr(pTmp3+1,TEXT('"'));
					else
						pTmp3 = pTmp1;
					if( pTmp4 )
						++pTmp3;
					else
						pTmp4 = StrChr(pTmp3,TEXT('\n'));

					if( pTmp4 && pTmp4-pTmp3>1 )
					{
						bSucc = true;
						pTmp1 = pTmp4;
					}
				}				
			}
			else
				bSucc = true;

			if( bSucc )
			{
				while( (pTmp2=StrStr(pTmp1+lstrlen(TEXT("\n")),TEXT("\n")))!=NULL ) /*eigentliche Abschnittsdaten fangen nach Leerzeile an*/
				{
					if( pTmp1+lstrlen(TEXT("\n"))==pTmp2 )
						break;
					pTmp1 = pTmp2;
				}
			}
			else
				pTmp2=NULL;
			
			if( pTmp2!=NULL ) /*Erfolgreiches Überlesen der Meta-Daten des Abschnitts*/
			{
				pTmp1=pTmp2+lstrlen(TEXT("\n"));
				pTmp2=pPartEnd;
				if( pTmp2>pTmp1 ) /*Abschnitt enthält mindestens ein Nutzdatum*/
				{
					prgTmpSection = bAttach ? prgAttachSection : prgPlainSection;
					pncTmpSection = bAttach ? pncAttachSection : pncPlainSection;

					bSucc = true;
					if( bAttach && ncPathPrefix )
					{
						bSucc = false;
						pTmpMtx = (TCHAR**) generic_realloc(*prgAttachPath,(*pncAttachPath+1)*sizeof(TCHAR*));
						if( *prgAttachPath )
						{
							*prgAttachPath = pTmpMtx;
							(*prgAttachPath)[*pncAttachPath] = generic_malloc((ncPathPrefix+pTmp4-pTmp3+2)*sizeof(TCHAR));
							if( (*prgAttachPath)[0] )
							{
								bSucc = true;

								lstrcpy((*prgAttachPath)[*pncAttachPath],szAttachPath);
								i = ncPathPrefix;
								if( szAttachPath[i-1]!=TEXT('\\') )
									lstrcpy((*prgAttachPath)[*pncAttachPath]+(i++),TEXT("\\"));
								for(; pTmp3<pTmp4; ++pTmp3,++i)
									(*prgAttachPath)[*pncAttachPath][i] = *pTmp3;
								(*prgAttachPath)[*pncAttachPath][i] = TEXT('\0');

								++(*pncAttachPath);
							}
						}
					}

					if( bSucc )
					{
						pTmpVec = (long*) generic_realloc(*prgTmpSection,(*pncTmpSection+1)*sizeof(long));
						if( pTmpVec==NULL )
						{
							--(*pncAttachPath);
							generic_free((*prgAttachPath)[*pncAttachPath]);
						}
					}
					else
						pTmpVec = NULL;

					if( pTmpVec!=NULL )
					{
						*prgTmpSection = pTmpVec;
						pTmpVec[*pncTmpSection] = MAKELONG(pTmp1-szMailBody,pTmp2-szMailBody);
						++(*pncTmpSection);
					}
				}
			}
		}

		*pPartEnd = c;
		pPartBeg = pPartEnd; 
	}
	
	if( *pncPlainSection==0 )
	{
		generic_free(*prgPlainSection);
		*prgPlainSection = NULL;
	}
	if( *pncAttachSection==0 )
	{
		generic_free(*prgAttachSection);
		*prgAttachSection = NULL;
	}
	if( *pncAttachPath==0 )
	{
		generic_free(*prgAttachPath);
		*prgAttachPath = NULL;
	}
	generic_free(szPartDelim);

	return *pncPlainSection>=0 || *pncAttachSection>=0;
}

static int ReadMailArray(TCHAR* szLine, TCHAR cDelim, TCHAR*** pDestArray)
{
	TCHAR *pTmp1, *pTmp2, *pTagBeg, *pTagEnd, **pTmpVec;
	int i;
	bool bSucc;

	for(pTmp1=szLine,i=1; (pTmp1=StrChr(pTmp1,cDelim))!=NULL; ++pTmp1,++i);
	if( *pDestArray )
	{
		pTmpVec = (TCHAR**) generic_realloc(*pDestArray,i*sizeof(TCHAR*));
		if( pTmpVec==NULL )
		{
			generic_free(*pDestArray);
			*pDestArray = NULL;
			return 0;
		}
		
		*pDestArray = pTmpVec;
	}
	else
	{
		*pDestArray = (TCHAR**) generic_malloc(i*sizeof(TCHAR*));
		if( *pDestArray==NULL )
				return 0;
	}
			
	for(pTagBeg=szLine,i=0; (pTagEnd=StrChr(pTagBeg,cDelim))!=0; pTagBeg=pTagEnd+1,++i)	
	{
		bSucc = (pTmp1=StrChr(pTagBeg,TEXT('<')))!=NULL;
		bSucc = bSucc && (pTmp2=StrChr(pTmp1+1,TEXT('>')))!=NULL;
		if( bSucc )
		{
			pTagBeg = pTmp1+1;
			pTagEnd = pTmp2;
		}
		PtrCpy(*pDestArray+i,pTagBeg,pTagEnd);
	}
	bSucc = (pTmp1=StrChr(pTagBeg,TEXT('<')))!=NULL;
	bSucc = bSucc && (pTmp2=StrChr(pTmp1+1,TEXT('>')))!=NULL;
	if( bSucc )
	{
		++pTmp1;
	}
	else
	{
		pTmp1 = pTagBeg;
		pTmp2 = szLine+lstrlen(szLine);
		for(; _istspace(*(pTmp2-1)); --pTmp2);
	}
	PtrCpy(*pDestArray+i,pTmp1,pTmp2);
	
	return ++i;
}

static bool ReadMailField(TCHAR* szLine, bool bBrackets, TCHAR** szDest1, TCHAR** szDest2)
{
	TCHAR *pTagBeg=NULL, *pTagEnd=NULL;
	bool bSucc;

	bSucc = bBrackets;
	bSucc = bSucc && (pTagBeg=StrChr(szLine,TEXT('<')))!=NULL;
	bSucc = bSucc && (pTagEnd=StrChr(pTagBeg+1,TEXT('>')))!=NULL;
	if( bSucc )
	{
		PtrCpy(szDest1,pTagBeg+1,pTagEnd);
		if( szDest2 )
		{
			for(pTagEnd=pTagBeg; _istspace(*(pTagEnd-1)); --pTagEnd);
			for(pTagBeg=szLine; _istspace(*pTagBeg); ++pTagBeg);
			if( *pTagBeg==TEXT('"') && *(pTagEnd-1)==TEXT('"') )
			{
				++pTagBeg;
				--pTagEnd;
			}
			PtrCpy(szDest2,pTagBeg,pTagEnd);
		}
	}
	else
	{
		for(pTagBeg=szLine; _istspace(*pTagBeg); ++pTagBeg);
		for(pTagEnd=szLine+lstrlen(szLine); _istspace(*(pTagEnd-1)); --pTagEnd);
		if( *pTagBeg==TEXT('"') && *(pTagEnd-1)==TEXT('"') )
		{
			++pTagBeg;
			--pTagEnd;
		}
		
		PtrCpy(szDest1,pTagBeg,pTagEnd);
		if( szDest2 && *szDest2 )
		{
			generic_free(*szDest2);
			*szDest2 = NULL;
		}
	}

	return bSucc;
}

static TCHAR* PtrCpy(TCHAR** szDest, TCHAR* pTagBeg, TCHAR* pTagEnd)
{
	unsigned int i;
	TCHAR* pLoop;

	if( *szDest )
	{
		generic_free(*szDest);
		*szDest = NULL;
	}

	if( pTagBeg==NULL || pTagEnd==NULL || (pTagEnd-pTagBeg)<0 )
		return NULL;

	*szDest = (TCHAR*) generic_malloc((pTagEnd-pTagBeg+1)*sizeof(TCHAR));
	if( *szDest==NULL )
		return NULL;

	for(pLoop=pTagBeg,i=0; pLoop<pTagEnd; ++pLoop,++i)
			(*szDest)[i] = *pLoop;
	(*szDest)[i] = TEXT('\0');

	return *szDest;
}

static char* StrCatDynA(char** szDest, size_t* StrSize, const char* szAppend)
{
	size_t l1, l2;
	char *ptr=NULL;

	l1 = *szDest ? strlen(*szDest) : 0;
	l2 = strlen(szAppend);
	if( *StrSize <= l1+l2 )
	{
		ptr = (char*) generic_realloc(*szDest,3*(l1+l2+1)/2*sizeof(char));
		if( ptr==NULL )
			return NULL;

		*szDest = ptr;
		*StrSize = 3*(l1+l2+1)/2; 
	}

	strcpy(*szDest+l1,szAppend);
	return *szDest;
}

