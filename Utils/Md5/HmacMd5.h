#ifndef HMAC_MD5_H_INCLUDED
#define HMAC_MD5_H_INCLUDED

int hmac_md5(unsigned char* text, int text_len, unsigned char* key, int key_len, unsigned char* digest);

#endif

