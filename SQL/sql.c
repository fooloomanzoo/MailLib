/***********************************************************************************/
/* Autor:  Markus Holtkötter               Telefon: 3082                           */
/* E-Mail: m.holtkoetter@fz-juelich.de           Zeitraum: 2008                    */
/*                                                                                 */
/* ergänzt von Werner Hürttlen Tel. 4357 2.11.2007                                 */
/* geändert von Tobias vonden Driesch Tel. 2906  13.05.2009						   */
/*					- SQL_Statement der Init-Funktion optimiert				       */
/*                                                                                 */
/*                                                                                 */
/***********************************************************************************/
/* SQL-Modul: Stellt über eine Dll Methoden zum Zugriff auf eine Datenbank bereit  */
/*                                                                                 */
/***********************************************************************************/
/* Programmiersprache: C        *                                                  */
/*                              * Rechner:  PC INTEL PENTIUM 4 3 GHz               */
/*                              *                                                  */
/* Compiler: Visual C++         *                                                  */
/*      Version 2005            * Betriebssystem: Windows XP SP2                   */
/*                                                                                 */
/***********************************************************************************/
/* Abhängigkeiten:																   */
/*                                                                                 */
/***********************************************************************************/

#include <windows.h>
#include <stdio.h>

#define MAIL_DLL_EXPORTS
// #include "MailDll.h"
#include "ibnsql.h"


HENV hEnv;
HDBC hDBC;
char** output;
int werte = 0, intervall = 0, filter = 0;
long rows = 0;
char* table;



/***********************************************************************************************/
/* Datenbankfunktionen (Verbindung, Eintragen, Ausgeben, ... )                                 */
/***********************************************************************************************/
/* Author: Markus Holtkötter                         IBN-TA-E                                  */
/* Email: m.holtkoetter@fz-juelich.de                Tel.:02461-61-2906                        */
/* Geb.: 02.4V         Raum: 0.15                                                              */
/***********************************************************************************************/
/*                                                                                             */
/***********************************************************************************************/


/***********************************************************************************************/
/* history_get_rows                                                                            */
/*   bestimmten (Durchschnitts-)Wert auslesen                                                  */
/***********************************************************************************************/
/* Parameter:     int wert         (Index des gewünschten Wertes, '0' ist der jüngste Wert)    */
/*                char*** output   (Pointer auf Ergebnis-Matrix)                               */
/* Rückgabewert:  FALSE   bei fehlerhaften SQL-Anfragen / Allocations-Fehlern                  */
/*                TRUE    bei erfolgreichem Ablauf                                             */
/***********************************************************************************************/
MAIL_DLL_EXPORT BOOL history_get_rows(int wert, char*** output) {
	RETCODE retCode;
	HSTMT hStmt;	// das Handle fuer die Anweisung
	char query[1024], query2[1024];
	char s[10], e[10];
	int i;
	char *help;
	SQLINTEGER   cbLen;
	SQLSMALLINT NumCols;

	help = (char*)calloc(30, sizeof(char));

	retCode = SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hStmt);
	if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
		free(help);
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		return FALSE;
	}

	itoa(rows - intervall*(wert + 1) + 1, s, 10);
	itoa(rows - intervall*wert, e, 10);

	// keine Berechnung wenn das Intervall aus einem Wert besteht
	if (intervall == 1) {
		strcpy(query, "SELECT time,gas1,gas2,gas3,gas4,alarm1,alarm2,stoerung FROM ordered WHERE R BETWEEN ");
	}
	else {
		// Welcher Wert aus dem Intervall wird benötigt? MAX(),MIN(),AVG()
		if (filter == F_EPITAXIE) {
			strcpy(query, "SELECT MAX(time),MAX(gas1),MAX(gas2),MAX(gas3),MAX(gas4),MIN(alarm1),MIN(alarm2),MIN(stoerung) FROM ordered WHERE R BETWEEN ");
		}
	}
	strcat(query, s);
	strcat(query, " AND ");
	strcat(query, e);

	//Abfrage ausführen
	retCode = SQLExecDirect(hStmt, (UCHAR*)query, SQL_NTS);
	if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
		free(help);
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		return FALSE;
	}

	// Ergebnis abspeichern
	SQLNumResultCols(hStmt, &NumCols);
	retCode = SQLFetch(hStmt);
	if (retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO) {
		for (i = 0; i < NumCols; i++) {
			if (SQLGetData(hStmt, i + 1, SQL_C_CHAR, help, 30, &cbLen) == SQL_SUCCESS) {
				if (help[0] == '.')
					strcat((*output)[wert], "0");
				else if ((help[0] == '-') && (help[1] == '.')) {
					strcat((*output)[wert], "-");
					help[0] = '0';
				}
				strcat((*output)[wert], help);
				strcat((*output)[wert], ";");
			}
			else {
				free(help);
				SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
				return FALSE;
			}
		}
	}
	else {
		free(help);
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		return FALSE;
	}
	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	free(help);
	return TRUE;
}

/***********************************************************************************************/
/* history_init                                                                                */
/*   Anzahl von Daten auslesen z.B. die letzten 1000 Werte                                     */
/***********************************************************************************************/
/* Parameter:     char* tbl        (Tabellenname)                                              */
/*                int anz_werte    (Anzahl der Werte)                                          */
/*                int iv           (Anzahl der Messwerte, aus denen ein Wert gebildet werden   */
/*                                  soll z.B.60 für Minutenwerte)                              */
/*                int ft           (Filter der verwendet werden soll z.B. MAX)                 */
/* Rückgabewert:  FALSE   bei fehlerhaften SQL-Anfragen / Allocations-Fehlern                  */
/*                TRUE    bei erfolgreichem Ablauf                                             */
/***********************************************************************************************/
MAIL_DLL_EXPORT BOOL history_init(char* tbl, int anz_werte, int iv, int ft) {
	int i;
	RETCODE retCode;
	HSTMT hStmt;	// das Handle fuer die Anweisung
	char query[1024];
	char e[10];

	// globale Variablen setzen
	werte = anz_werte;
	intervall = iv;
	rows = count_row(tbl);
	table = tbl;
	filter = ft;

	itoa(rows - intervall*werte, e, 10);

	output = (char**)calloc(werte, sizeof(char*));
	for (i = 0; i < werte; i++) {
		output[i] = (char*)calloc(1024, sizeof(char));
	}

	retCode = SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hStmt);
	if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		return FALSE;
	}

	// benötigte Werte sortiert in seperate Tablle schreiben
	strcpy(query, "CREATE TABLE ORDERED AS (select * from (select ");
	strcat(query, table);
	strcat(query, ".*, rownum as R from ");
	strcat(query, table);
	strcat(query, " ) where R > ");
	strcat(query, e);
	strcat(query, ")");

	/*strcpy(query, "CREATE table ordered AS SELECT * FROM (SELECT ");
	strcat(query, table);
	strcat(query, ".*, ROW_NUMBER() OVER (ORDER BY time) R FROM ");
	strcat(query, table);
	strcat(query,") WHERE R > ");
	strcat(query,e);
	*/

	/************************************************/
	//Ergaenzung/Alternative:	Tobias von den Driesch
	//							03.02.2009
	//
	/************************************************/
	//strcpy(query, "CREATE TABLE ordered as Select ");
	//strcat(query, table);
	//strcat(query, ".*, rownum as R from ");
	//strcat(query, table);
	//strcat(query, " WHERE R > ");
	//strcat(query, e);

	//Abfrage ausführen
	retCode = SQLExecDirect(hStmt, (UCHAR*)query, SQL_NTS);
	if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		return FALSE;
	}

	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	return TRUE;
}

/***********************************************************************************************/
/* history_get                                                                                 */
/*   Einen Wert aus der History zurückgeben                                                    */
/*   Historische Daten auf Feld gespeichert                                                    */
/***********************************************************************************************/
/* Parameter:     int wert         (gewünschter Wert)                                          */
/*                char* record     (Ergebnis-Array)                                            */
/* Rückgabewert:  FALSE   wenn ein nicht vorhandener Wert ausgelesen werden soll               */
/*                TRUE    bei erfolgreichem Ablauf                                             */
/***********************************************************************************************/
MAIL_DLL_EXPORT BOOL history_get(int wert, char* record) {

	*record = 0;
	if (wert < werte) {
		// Wert berechnen
		history_get_rows(wert, &output);
		sprintf(record, output[wert]);
		return TRUE;
	}
	else
		return FALSE;
}

/***********************************************************************************************/
/* history_destroy                                                                             */
/*   Speicherplatz freigeben                                                                   */
/***********************************************************************************************/
/* Parameter:     -                                                                            */
/* Rückgabewert:  void                                                                         */
/***********************************************************************************************/
MAIL_DLL_EXPORT void history_destroy() {
	int i;
	RETCODE retCode;
	HSTMT hStmt;	// das Handle fuer die Anweisung
	char query[1024];

	for (i = 0; i < werte; i++) {
		free(output[i]);
	}
	free(output);

	retCode = SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hStmt);
	if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	}

	strcpy(query, "DROP table ordered");

	//Abfrage ausführen
	retCode = SQLExecDirect(hStmt, (UCHAR*)query, SQL_NTS);
	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}


/***********************************************************************************************/
/* connect_to_database                                                                         */
/*   Verbindung zu einer Datenbank herstellen                                                  */
/***********************************************************************************************/
/* Parameter:     char* DB         (DSN Datenbank-System-Name)                                 */
/*                char* USER       (Datenbankaccount)                                          */
/*                char* PW         (Passwort)                                                  */
/* Rückgabewert:  FALSE   bei fehlerhaften SQL-Anfragen / Allocations-Fehlern                  */
/*                TRUE    bei erfolgreichem Ablauf                                             */
/***********************************************************************************************/

MAIL_DLL_EXPORT BOOL connect_to_database(char* DB, char* USER, char* PW) {
	RETCODE retCode;

	if (!hEnv) {
		SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
		SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
		SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDBC);
	}
	if (hDBC) {
		//mit Datenbank verbinden
		retCode = SQLConnect(hDBC, (UCHAR *)DB, SQL_NTS, (UCHAR *)USER, SQL_NTS, (UCHAR *)PW, SQL_NTS);
		if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
			SQLFreeHandle(SQL_HANDLE_STMT, hDBC);
			SQLFreeHandle(SQL_HANDLE_STMT, hEnv);
			return FALSE;
		}
	}
	else {
		SQLFreeHandle(SQL_HANDLE_STMT, hDBC);
		SQLFreeHandle(SQL_HANDLE_STMT, hEnv);
		return FALSE;
	}
	return TRUE;
}

/***********************************************************************************************/
/* execute_statement                                                                           */
/*   ein beliebiges SQL-Statement ausführen                                                    */
/***********************************************************************************************/
/* Parameter:     char* query      (SQL-Statement)                                             */
/* Rückgabewert:  FALSE   bei fehlerhaften SQL-Anfragen / Allocations-Fehlern                  */
/*                TRUE    bei erfolgreichem Ablauf                                             */
/***********************************************************************************************/
MAIL_DLL_EXPORT BOOL execute_stmt(char* query) {
	RETCODE retCode;
	HSTMT hStmt;	// das Handle fuer die Anweisung

	retCode = SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hStmt);
	if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		return FALSE;
	}
	//Abfrage ausführen
	retCode = SQLExecDirect(hStmt, (UCHAR*)query, SQL_NTS);
	if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		return FALSE;
	}
	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	return TRUE;
}

/***********************************************************************************************/
/* select_all                                                                                  */
/*   kompletten Inhalt einer Tabelle auslesen                                                  */
/***********************************************************************************************/
/* Parameter:     char* table      (Tabellenname)                                              */
/*                char*** output   (Pointer auf Ergebnis-Array)                                */
/* Rückgabewert:  FALSE   bei fehlerhaften SQL-Anfragen / Allocations-Fehlern                  */
/*                TRUE    bei erfolgreichem Ablauf                                             */
/***********************************************************************************************/
MAIL_DLL_EXPORT BOOL select_all(char* table, char*** output) {
	RETCODE retCode;
	HSTMT hStmt;	// das Handle fuer die Anweisung
	int anz_rows, i, j;
	char *help;
	SQLINTEGER   cbLen;
	SQLSMALLINT NumCols;
	char query[1024];

	strcpy(query, "Select * from ");
	strcat(query, table);
	strcat(query, " order by time");

	anz_rows = count_row(table);

	help = (char*)calloc(30, sizeof(char));

	retCode = SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hStmt);
	if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
		free(help);
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		return FALSE;
	}
	//Abfrage ausführen
	retCode = SQLExecDirect(hStmt, (UCHAR*)query, SQL_NTS);
	if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
		free(help);
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		return FALSE;
	}
	SQLNumResultCols(hStmt, &NumCols);

	//Ergebnis speichern
	for (j = 0; j < anz_rows; j++) {
		retCode = SQLFetch(hStmt);
		if (retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO) {
			for (i = 0; i < NumCols; i++) {
				if (SQLGetData(hStmt, i + 1, SQL_C_CHAR, help, 30, &cbLen) == SQL_SUCCESS) {
					free(help);
					if (help[0] == '.')
						strcat((*output)[j], "0");
					strcat((*output)[j], help);
					strcat((*output)[j], ";");
				}
				else {
					free(help);
					SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
					return FALSE;
				}
			}
		}
		else {
			free(help);
			SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
			return FALSE;
		}
	}
	free(help);
	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	return TRUE;
}

/***********************************************************************************************/
/* clear_table                                                                                 */
/*   Inhalt einer Tabelle löschen                                                              */
/***********************************************************************************************/
/* Parameter:     char* table      (Tabellenname)                                              */
/* Rückgabewert:  FALSE   bei fehlerhaften SQL-Anfragen / Allocations-Fehlern                  */
/*                TRUE    bei erfolgreichem Ablauf                                             */
/***********************************************************************************************/
MAIL_DLL_EXPORT BOOL clear_table(char* table) {
	RETCODE retCode;
	HSTMT hStmt;	// das Handle fuer die Anweisung
	char query[1024];

	strcpy(query, "TRUNCATE TABLE ");
	strcat(query, table);

	retCode = SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hStmt);
	if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		return FALSE;
	}
	//Tabelle leeren
	retCode = SQLExecDirect(hStmt, (UCHAR*)query, SQL_NTS);
	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
		return FALSE;
	}
	return TRUE;
}

/***********************************************************************************************/
/* count_row                                                                                   */
/*   Anzahl der Records in einer Tabelle bestimmen                                             */
/***********************************************************************************************/
/* Parameter:     char* table      (Tabellenname)                                              */
/* Rückgabewert:  Anzahl der Zeilen bzw: -1 bei Fehlern                                        */
/***********************************************************************************************/
MAIL_DLL_EXPORT int count_row(char* table) {
	RETCODE retCode;
	HSTMT hStmt;	// das Handle fuer die Anweisung
	char query[1024];
	SQLINTEGER count, len;

	strcpy(query, "SELECT COUNT(*) FROM ");
	strcat(query, table);

	retCode = SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hStmt);
	if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		return -1;
	}
	//Abfrage ausführen
	retCode = SQLExecDirect(hStmt, (UCHAR*)query, SQL_NTS);
	if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		return -1;
	}

	retCode = SQLFetch(hStmt);
	if (retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO) {
		SQLGetData(hStmt, 1, SQL_C_ULONG, &count, 0, &len);
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		return count;
	}
	else {
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		return -1;
	}
}

/***********************************************************************************************/
/* last_row                                                                                    */
/*   letzten Record auslesen                                                                   */
/***********************************************************************************************/
/* Parameter:     char* table      (Tabellenname)                                              */
/*                char* output     (Ergebnis-Array)                                            */
/* Rückgabewert:  FALSE   bei fehlerhaften SQL-Anfragen / Allocations-Fehlern                  */
/*                TRUE    bei erfolgreichem Ablauf                                             */
/***********************************************************************************************/
MAIL_DLL_EXPORT BOOL last_row(char* table, char* output) {
	RETCODE retCode;
	HSTMT hStmt;	// das Handle fuer die Anweisung
	int i;
	char *help;
	char query[1024], query2[1024];
	SQLINTEGER   cbLen;
	SQLSMALLINT NumCols;

	*output = 0;
	help = (char*)calloc(30, sizeof(char));

	/*strcpy(query, "SELECT * FROM ");
	strcpy(query2, " WHERE DBMS_ROWID.ROWID_ROW_NUMBER(ROWID) = (SELECT MAX(DBMS_ROWID.ROWID_ROW_NUMBER(ROWID)) FROM ");
	strcat(query, table);
	strcat(query, query2);
	strcat(query, table);
	strcat(query, ")");*/

	strcpy(query, "SELECT * FROM ");
	strcpy(query2, " WHERE time = (SELECT MAX(time) FROM ");
	strcat(query, table);
	strcat(query, query2);
	strcat(query, table);
	strcat(query, ")");

	retCode = SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hStmt);
	if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
		free(help);
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		return FALSE;
	}
	//Abfrage ausführen
	retCode = SQLExecDirect(hStmt, (UCHAR*)query, SQL_NTS);
	if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
		free(help);
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		return FALSE;
	}
	SQLNumResultCols(hStmt, &NumCols);

	// Ergebnis speichern
	retCode = SQLFetch(hStmt);
	if (retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO) {
		for (i = 0; i < NumCols; i++) {
			if (SQLGetData(hStmt, i + 1, SQL_C_CHAR, help, 30, &cbLen) == SQL_SUCCESS) {
				if (help[0] == '.')
					strcat(output, "0");
				else if ((help[0] == '-') && (help[1] == '.')) {
					strcat(output, "-");
					help[0] = '0';
				}
				strcat(output, help);
				strcat(output, ";");
			}
			else {
				free(help);
				SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
				return FALSE;
			}
		}
	}
	else {
		free(help);
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		return FALSE;
	}
	free(help);
	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	return TRUE;
}

/***********************************************************************************************/
/* get_rows                                                                                    */
/*   Bestimmte Records auslesen z.B. von 100-200                                               */
/***********************************************************************************************/
/* Parameter:     char* table      (Tabellenname)                                              */
/*                int start        (Startwert)                                                 */
/*                int ende         (Endwert)                                                   */
/*                char*** output   (Pointer auf Ergebnis-Array)                                */
/* Rückgabewert:  FALSE   bei fehlerhaften SQL-Anfragen / Allocations-Fehlern                  */
/*                TRUE    bei erfolgreichem Ablauf                                             */
/***********************************************************************************************/
MAIL_DLL_EXPORT BOOL get_rows(char* table, int start, int ende, char*** output) {
	RETCODE retCode;
	HSTMT hStmt;	// das Handle fuer die Anweisung
	char query[1024], query2[1024];
	char s[10], e[10];
	int anz_rows, i, j;
	char *help;
	SQLINTEGER   cbLen;
	SQLSMALLINT NumCols;

	help = (char*)calloc(30, sizeof(char));

	anz_rows = count_row(table);
	itoa(anz_rows - start + 1, e, 10);
	itoa(anz_rows - ende + 1, s, 10);

	// Statement erzeugen
	strcpy(query, "SELECT * FROM (SELECT ");
	strcat(query, table);
	strcat(query, ".*, ROW_NUMBER() OVER (ORDER BY time) R FROM ");
	strcat(query, table);
	strcat(query, ") WHERE R BETWEEN ");
	strcat(query, s);
	strcat(query, " AND ");
	strcat(query, e);

	retCode = SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hStmt);
	if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
		free(help);
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		return FALSE;
	}
	//Abfrage ausführen
	retCode = SQLExecDirect(hStmt, (UCHAR*)query, SQL_NTS);
	if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
		//Abfrage wenn nicht ORACLE verwendet wird.
		//Spalte 'time' notwendig
		itoa(anz_rows - start, e, 10);
		itoa(anz_rows - ende, s, 10);
		strcpy(query, "SELECT * FROM ");
		strcpy(query2, " ORDER BY time LIMIT ");
		strcat(query, table);
		strcat(query, query2);
		strcat(query, s);
		strcat(query, ",");
		strcat(query, e);
		retCode = SQLExecDirect(hStmt, (UCHAR*)query, SQL_NTS);
		if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
			free(help);
			SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
			return FALSE;
		}
	}
	SQLNumResultCols(hStmt, &NumCols);

	// Ergebnis speichern
	for (j = 0; j <= (ende - start); j++) {
		retCode = SQLFetch(hStmt);
		if (retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO) {
			for (i = 0; i < NumCols - 1; i++) {
				if (SQLGetData(hStmt, i + 1, SQL_C_CHAR, help, 30, &cbLen) == SQL_SUCCESS) {
					if (help[0] == '.')
						strcat((*output)[j], "0");
					else if ((help[0] == '-') && (help[1] == '.')) {
						strcat((*output)[j], "-");
						help[0] = '0';
					}
					strcat((*output)[j], help);
					strcat((*output)[j], ";");
				}
				else {
					free(help);
					SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
					return FALSE;
				}
			}
		}
		else {
			free(help);
			SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
			return FALSE;
		}
	}
	free(help);
	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	return TRUE;
}

/***********************************************************************************************/
/* delete_rows                                                                                 */
/*   bestimmte Records aus der Tabelle löschen                                                 */
/***********************************************************************************************/
/* Parameter:     char* table      (Tabellenname)                                              */
/*                int start        (Startwert)                                                 */
/*                int ende         (Endwert)                                                   */
/* Rückgabewert:  FALSE   bei fehlerhaften SQL-Anfragen / Allocations-Fehlern                  */
/*                TRUE    bei erfolgreichem Ablauf                                             */
/***********************************************************************************************/
MAIL_DLL_EXPORT BOOL delete_rows(char* table, int start, int ende) {
	RETCODE retCode;
	HSTMT hStmt;	// das Handle fuer die Anweisung
	char query[1024], query2[1024];
	char s[10], e[10];
	int anz_rows;

	anz_rows = count_row(table);
	itoa(anz_rows - start, e, 10);
	itoa(anz_rows - ende, s, 10);

	strcpy(query, "DELETE FROM ");
	strcpy(query2, " WHERE DBMS_ROWID.ROWID_ROW_NUMBER(ROWID) BETWEEN ");
	strcat(query, table);
	strcat(query, query2);
	strcat(query, s);
	strcat(query, " AND ");
	strcat(query, e);

	retCode = SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hStmt);
	if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		return FALSE;
	}
	//Abfrage ausführen
	retCode = SQLExecDirect(hStmt, (UCHAR*)query, SQL_NTS);
	if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
		//Abfrage wenn nicht ORACLE verwendet wird.
		//Spalte 'time' notwendig
		itoa(ende - 1, e, 10);
		itoa(start - 1, s, 10);
		strcpy(query, "DELETE FROM ");
		strcat(query, table);
		strcat(query, " WHERE time BETWEEN (SELECT time FROM (SELECT * FROM ");
		strcat(query, table);
		strcat(query, ") as X ORDER BY time DESC limit ");
		strcat(query, e);
		strcat(query, ",1) and (SELECT time FROM (SELECT * FROM ");
		strcat(query, table);
		strcat(query, ") as X ORDER BY time DESC limit ");
		strcat(query, s);
		strcat(query, ",1)");
		retCode = SQLExecDirect(hStmt, (UCHAR*)query, SQL_NTS);
		if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
			SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
			return FALSE;
		}
	}
	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	return TRUE;
}

/***********************************************************************************************/
/* disconnect_from_database                                                                    */
/*   Datenbankverbindung schliessen und Speicherplatz freigeben                                */
/***********************************************************************************************/
/* Parameter:     -                                                                            */
/* Rückgabewert:  FALSE   bei fehlgeschlagenem SQLDisconnect                                   */
/*                TRUE    bei erfolgreichem Ablauf                                             */
/***********************************************************************************************/
MAIL_DLL_EXPORT BOOL disconnect_from_database() {
	RETCODE retCode;
	retCode = SQLDisconnect(hDBC);
	if (retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO) {
		//SQLFreeHandle(SQL_HANDLE_DBC, hDBC);
		//SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		return TRUE;
	}
	else
		return FALSE;
}

