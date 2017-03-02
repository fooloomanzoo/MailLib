#ifndef IBNSQL_H_INCLUDED
#define IBNSQL_H_INCLUDED

#ifdef MAIL_DLL_EXPORTS
#define MAIL_DLL_EXPORT __declspec(dllexport)
//#define DLL_FUNC extern "C" __declspec(dllexport)
#else
#define MAIL_DLL_EXPORT __declspec(dllimport)
#endif

#include "../MailLib.h"

#include <windows.h>
#include <sql.h>
#include <sqlext.h>

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// Filter
#define F_EPITAXIE 1
	
extern HENV hEnv;
extern HDBC hDBC;

/*Funktionen zum Datenbankzugriff*/

MAIL_DLL_EXPORT BOOL connect_to_database(char* DB,char* USER,char* PW);				//Verbindung herstellen
MAIL_DLL_EXPORT BOOL execute_stmt(char* query);										//Statement ausführen
MAIL_DLL_EXPORT BOOL select_all(char* table,char*** output);
MAIL_DLL_EXPORT BOOL clear_table(char* table);										//Tabelle leeren
MAIL_DLL_EXPORT int  count_row(char* table);										//Anzahl Records
MAIL_DLL_EXPORT BOOL last_row(char* table,char* output);                            //letzter Eintrag
MAIL_DLL_EXPORT BOOL get_rows(char* table,int start, int ende,char*** output);      //Eintrag von ... bis ...
MAIL_DLL_EXPORT BOOL delete_rows(char* table,int start, int ende);					//löscht Eintrag von ... bis ...
MAIL_DLL_EXPORT BOOL disconnect_from_database();									//Verbindung trennen
MAIL_DLL_EXPORT BOOL history_init(char* table,int anz_werte,int iv,int filter);		//Initialisiert Datenbank für History-Zugriff
MAIL_DLL_EXPORT BOOL history_get(int wert,char* record);							//liefert den gewünschten Wert zurück
MAIL_DLL_EXPORT BOOL history_get_rows(int wert, char*** output);					//wird von history_get benötigt
MAIL_DLL_EXPORT void history_destroy();												//Speicherplatz freigeben

#ifdef __cplusplus
}
#endif

#endif
