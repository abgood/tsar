#ifndef TSAR_AES_CODE_H
#define TSAR_AES_CODE_H

/* AES加密 */
void aes_enc(char *);

/* AES解密 */
char *aes_dec(char *, int);

/* byte转换16进制字符串 */
void byte2hexstr(char *, unsigned char *, int);

/* 16进制字符串转换成byte */
void hexstr2byte(unsigned char *, char *, int);

#endif
