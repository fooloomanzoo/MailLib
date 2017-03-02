#ifndef BASE_64_H_INCLUDED
#define BASE_64_H_INCLUDED

int B64Enc(char** pB64Str, const unsigned char* rgEncBuf, int ncEncBuf);
/*Wandelt Byte-Stream rgEncBuf der Größe ncEncBuf-Bytes in nullterminierten
Base64-codierte US-ASCII String *pB64Str um, für den der benötigte Speicher angelegt wird.
Funktion gibt die Länge des codierten Strings zurück, -2 bei fehlerhaften Parametern,
-1 bei unzurückendem Speicher*/
/*erfordert manuelle Speicherfreigabe mittels free*/

int B64Dec(unsigned char** pByteStream, const char* szDecBuf);
/*Wandelt Base64-codierten, nullterminierten US-ASCII String in
ursprünglichen Byte-Stream *pByteStream um, für den der benötigte Speicher angelegt wird.
Es wird zusätzlich ein Null-Byte angehängt aus Kompatibilität zu C-Strings.
Funktion gibt die Länge der decodierten Byte-Stream zurück, -3 bei fehlerhaften Parametern,
-2 bei unzureichendem Speicher, -1 bei ungültigem Base64-codierten Stream*/
/*erfordert manuelle Speicherfreigabe mittels free*/

#endif
