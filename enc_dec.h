#ifndef TSAR_ENC_DEC_H
#define TSAR_ENC_DEC_H

/* 加密字符串 */
void enc_code(char *);

/* 解密字符串 */
char *dec_code(char *, int);

/* 字节转换成位 */
void byte2bit(bool *, char *, int);

/* 表置换 */
void table_permute(bool *, bool *, const char *, int);

/* 位拷贝 */
void bits_copy(bool *, bool *, int);

/* 子密钥移位 */
void loop_move(bool *, int, int);

/* 执行des加密 */
void enc_des(char *, char *);

/* 执行des解密 */
void dec_des(char *, char *);

/* F函数变换 */
void F_change(bool *, bool *);

/* 异或 */
void Xor(bool *, bool *, int);

/* S盒变换, 输入48位,输出32位 */
void S_change(bool *, bool *);

/* 2进制密文转换为16进制 */
void bit2hex(char *, bool *, int);

/* 16进制密文转换为2进制 */
void hex2bit(bool *, char *, int);

/* 2进制位转换成字节 */
void bit2byte(char *, bool *, int);

#endif
