#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static const char base64digits[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define BAD     -1
static const char base64val[] = {
    BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD,
    BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD,
    BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD, 62, BAD,BAD,BAD, 63,
     52, 53, 54, 55,  56, 57, 58, 59,  60, 61,BAD,BAD, BAD,BAD,BAD,BAD,
    BAD,  0,  1,  2,   3,  4,  5,  6,   7,  8,  9, 10,  11, 12, 13, 14,
     15, 16, 17, 18,  19, 20, 21, 22,  23, 24, 25,BAD, BAD,BAD,BAD,BAD,
    BAD, 26, 27, 28,  29, 30, 31, 32,  33, 34, 35, 36,  37, 38, 39, 40,
     41, 42, 43, 44,  45, 46, 47, 48,  49, 50, 51,BAD, BAD,BAD,BAD,BAD
};
#define DECODE64(c)  (isascii(c) ? base64val[c] : BAD)

int B64Enc(char** pB64Str, const unsigned char* rgEncBuf, int ncEncBuf)
{
	int nOutLen, nDecIdx=0;

    if( pB64Str==NULL || rgEncBuf==NULL || ncEncBuf<0 )
		return -2;

	nOutLen = ncEncBuf/3;
	if( ncEncBuf%3 )
		nOutLen+= 3-ncEncBuf%3;
	nOutLen*= 4;
	*pB64Str = calloc(nOutLen+1,sizeof(char));
	if( *pB64Str==NULL )
		return -1;

	for(; ncEncBuf>=3; ncEncBuf-=3)
        {
				(*pB64Str)[nDecIdx++] = base64digits[rgEncBuf[0] >> 2];
                (*pB64Str)[nDecIdx++] = base64digits[((rgEncBuf[0] << 4) & 0x30) | (rgEncBuf[1] >> 4)];
                (*pB64Str)[nDecIdx++] = base64digits[((rgEncBuf[1] << 2) & 0x3c) | (rgEncBuf[2] >> 6)];
                (*pB64Str)[nDecIdx++] = base64digits[rgEncBuf[2] & 0x3f];
                rgEncBuf+= 3;
        }

        if( ncEncBuf>0 )
        {
                unsigned char fragment;

                (*pB64Str)[nDecIdx++] = base64digits[rgEncBuf[0] >> 2];
                fragment = (rgEncBuf[0] << 4) & 0x30;

                if( ncEncBuf>1 )
                        fragment |= rgEncBuf[1] >> 4;

                (*pB64Str)[nDecIdx++] = base64digits[fragment];
                (*pB64Str)[nDecIdx++] = (ncEncBuf<2) ? '=' : base64digits[(rgEncBuf[1] << 2) & 0x3c];
                (*pB64Str)[nDecIdx++] = '=';
        }

	return nDecIdx;
}

int B64Dec(unsigned char** pByteStream, const char* szDecBuf)
{
        int nOutIdx, nOutLen;
        register unsigned char digit1, digit2, digit3, digit4;

		if( pByteStream==NULL || szDecBuf==NULL )
			return -3;
		
		nOutLen = nOutIdx = strlen(szDecBuf);
		if( nOutLen%4!=0 )
			return -3;
		if( nOutLen==0 )
			return 0;

		if( szDecBuf[nOutIdx-1]=='=' )
		{
			--nOutIdx;
			if( nOutIdx>0 && szDecBuf[nOutIdx-1]=='=' )
				--nOutIdx;
		}
		nOutLen = (nOutLen/4)*3-(nOutLen-nOutIdx);
		/*Allokation eines zusätzlichen Byte zur Kompatibilität eines evtl. decodierten Strings*/
		*pByteStream = calloc(nOutLen+1,sizeof(unsigned char) );
		if( *pByteStream==NULL )
			return -2;
		nOutIdx = 0;

        do {
                digit1 = szDecBuf[0];
                if( DECODE64(digit1)==BAD )
				{
					free(*pByteStream);
					*pByteStream = NULL;
                    return -1;
				}
                digit2 = szDecBuf[1];
                if( DECODE64(digit2)==BAD )
				{
					free(*pByteStream);
					*pByteStream = NULL;
                    return -1;
				}
                digit3 = szDecBuf[2];
                if( digit3!='=' && DECODE64(digit3)==BAD )
				{
					free(*pByteStream);
					*pByteStream = NULL;
                    return -1;
				}
                digit4 = szDecBuf[3];
                if( digit4!='=' && DECODE64(digit4)==BAD )
				{
					free(*pByteStream);
					*pByteStream = NULL;
                    return -1;
				}
                szDecBuf += 4;
				
                (*pByteStream)[nOutIdx++] = (DECODE64(digit1) << 2) | (DECODE64(digit2) >> 4);
                  if( digit3!='=' )
                {
                        (*pByteStream)[nOutIdx++] = ((DECODE64(digit2) << 4) & 0xf0) | (DECODE64(digit3) >> 2);
                           if( digit4!='=' )
                                (*pByteStream)[nOutIdx++] = ((DECODE64(digit3) << 6) & 0xc0) | DECODE64(digit4);
                }
        } while( *szDecBuf && digit4!='=' );

        return nOutIdx;
}
