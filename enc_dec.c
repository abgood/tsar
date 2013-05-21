#include "tsar.h"
#include "enc_dec_table.h"

/* 字节转换成位 
 * 8次换一个字节 每次向右移一位
 * 和1与取最后一位 共64位
 */
void byte2bit(bool *dest, char *src, int len) {
    int i;

    for (i = 0; i < len; i++)
        dest[i] = (src[i / 8] >> (i % 8)) & 0x01;
}

/* 2进制密文转换为16进制 */
void bit2hex(char *dest, bool *src, int len) {
    int i;

    for (i = 0; i < len / 4; i++)
        dest[i] = 0;

    for (i = 0; i < len / 4; i++) {
        dest[i] = src[i * 4] + (src[i * 4 + 1] << 1) + (src[i * 4 + 2] << 2) + (src[i * 4 + 3] << 3);
        if ((dest[i] % 16) > 9)
            dest[i] = dest[i] % 16 + '7';
        else
            dest[i] = dest[i] % 16 + '0';
    }
}

/* 16进制转换成2进制 */
void hex2bit(bool *dest, char *src, int len) {
    int i;
    
    for (i = 0; i < len; i++) {
        if (src[i / 4] > '9')
            dest[i] = ((src[i / 4] - '7') >> (i % 4)) & 0x01;
        else
            dest[i] = ((src[i / 4] - '0') >> (i % 4)) & 0x01;
    }
}

/* 2进制位转换成字节 */
void bit2byte(char *dest, bool *src, int len) {
    int i;

    for (i = 0; i < (len / 8); i++)
        dest[i] = 0;

    for (i = 0; i < len; i++)
        dest[i / 8] |= src[i] << (i % 8);       /* 或等于 */
}

/* 异或 */
void Xor(bool *dataA, bool *dataB, int len) {
    int i;

    for (i = 0; i < len; i++)
        dataA[i] ^= dataB[i];
}

/* 位拷贝, src长度为len数组拷贝到dest里 */
void bits_copy(bool *dest, bool *src, int len) {
    int i;

    for (i = 0; i < len; i++)
        dest[i] = src[i];
}

/* 表置换 */
void table_permute(bool *dest, bool *src, const char *table, int len) {
    int i;
    static bool temp[LEN_256] = {0};

    for (i = 0; i < len; i++)
        temp[i] = src[table[i] - 1];
    /* 位拷贝 */
    bits_copy(dest, temp, len);
}

/* 子密钥移位, 循环左移 */
void loop_move(bool *src, int len, int num) {
    static bool temp[LEN_256] = {0};

    bits_copy(temp, src, num);              /* src最左边的num位移入temp里 */
    bits_copy(src, src + num, len - num);   /* 移入到temp里的剩下部分移到src最开始的位置 */
    bits_copy(src + len - num, temp, num);  /* temp里的数据移回到src的最后面 */
}

/* 设置密钥 */
void set_key(char *key) {
    int i;
    static bool key_bit[LEN_64] = {0};      /* 密钥二进制存储空间 */
    static bool *kil = &key_bit[0];         /* 前28位 */
    static bool *kir = &key_bit[28];        /* 后28位 */

    byte2bit(key_bit, key, LEN_64);         /* 密钥转换为二进制 */
    table_permute(key_bit, key_bit, PC1_table, 56);     /* PC1表置换56次 */

    for (i = 0; i < 16; i++) {
        loop_move(kil, 28, move_table[i]);      /* 前28位左移 */
        loop_move(kir, 28, move_table[i]);      /* 后28位左移 */

        /* 二维数组sub_key[i]为每一行起始地址
         * 每移一次位进行PC2置换得 Ki 48位
         */
        table_permute(sub_key[i], key_bit, PC2_table, 48);
    }
}

/* S盒变换 */
void S_change(bool dest[LEN_32], bool src[48]) {
    int i, X, Y;

    /* i为8个S盒
     * 每执行一次,输入数据偏移6位
     * 每执行一次,输出数据偏移4位
     * af代表第几行
     * bcde代表第几列
     * 把找到的点数据换为二进制
     */
    for (i = 0, X = 0, Y = 0; i < 8; i++, src += 6, dest += 4) {
        Y = (src[0] << 1) + src[5];
        X = (src[1] << 3) + (src[2] << 2) + (src[3] << 1) + src[4];
        byte2bit(dest, &S_box[i][Y][X], 4);
    }
}

/* F函数变换 */
void F_change(bool src[LEN_32], bool srcKi[48]) {
    static bool mir[48] = {0};

    table_permute(mir, src, E_table, 48);       /* E表置换48次 */
    Xor(mir, srcKi, 48);                        /* 和子密钥异或 */
    S_change(src, mir);                         /* S盒变换 */
    table_permute(src, src, P_table, LEN_32);   /* P表置换 */
}

/* 执行des加密 */
void enc_des(char dest[LEN_8], char src[LEN_8]) {
    int i;
    static bool clear_bit[LEN_64] = {0};        /* 明文二进制 */
    static bool temp[LEN_32] = {0};
    static bool *mil = &clear_bit[0];           /* 前32位 */
    static bool *mir = &clear_bit[LEN_32];      /* 后32位 */

    byte2bit(clear_bit, src, LEN_64);           /* 明文转换成二进制位 */
    table_permute(clear_bit, clear_bit, IP_table, LEN_64);      /* ip表置换 */

    /* 迭代16次 */
    for (i = 0; i < LEN_16; i++) {
        bits_copy(temp, mir, LEN_32);
        F_change(mir, sub_key[i]);          /* F函数变换 */
        Xor(mir, mil, LEN_32);              /* 得到Ri */
        bits_copy(mil, temp, LEN_32);       /* 得到Li */
    }
    table_permute(clear_bit, clear_bit, IPR_table, LEN_64);     /* IPR表置换 */
    bit2hex(dest, clear_bit, LEN_64);       /* 2进制密文转换为16进制 */
}

/* 执行des解密 */
void dec_des(char dest[LEN_8], char src[LEN_8]) {
    int i;
    static bool encry_bit[LEN_64] = {0};        /* 密文二进制 */
    static bool temp[LEN_32] = {0};
    static bool *mil = &encry_bit[0];           /* 前32位 */
    static bool *mir = &encry_bit[LEN_32];      /* 后32位 */

    hex2bit(encry_bit, src, LEN_64);            /* 密文转换成二进制位 */
    table_permute(encry_bit, encry_bit, IP_table, LEN_64);      /* ip表置换 */

    /* 迭代16次 */
    for (i = 15; i >= 0; i--) {
        bits_copy(temp, mil, LEN_32);
        F_change(mil, sub_key[i]);          /* F函数变换 */
        Xor(mil, mir, LEN_32);              /* 得到Li */
        bits_copy(mir, temp, LEN_32);       /* 得到Ri */
    }
    table_permute(encry_bit, encry_bit, IPR_table, LEN_64);     /* IPR表置换 */
    bit2byte(dest, encry_bit, LEN_64);       /* 2进制明文转换为字符串 */
}

/* 加密字符串 */
void enc_code(char *string) {
    int i;
    char key[LEN_8] = {0};
    char pawd[16] = {0};

    /* 待加密字符串 */
    if (strlen(string) < 8)
        do_debug(LOG_FATAL, "Encrypt string length less than 8 bytes\n");
    else
        string[LEN_8] = '\0';
    /* 密钥 */
    strncat(key, "love7road", LEN_8);

    /* 设置密钥, 得到子密钥Ki */
    set_key(key);

    /* 执行des加密 */
    enc_des(pawd, string);

    /* 打印加密字符串 */
    printf("Your Message is Encrypted: ");
    for (i = 0; i < LEN_16; i++)
        printf("%c", pawd[i]);
    printf("\n");
}

/* 解密字符串 */
char *dec_code(char *pawd, int is_print) {
    int i;
    char key[LEN_8] = {0};
    /* 动态分配内存, 地址是有效的 */
    char *string = (char *)calloc(LEN_8, sizeof(char *));

    /* 密钥 */
    strncat(key, "love7road", LEN_8);
    
    /* 设置密钥, 得到子密钥Ki */
    set_key(key);

    /* 执行des解密 */
    dec_des(string, pawd);

    /* 打印解密字符串 */
    if (is_print) {
        printf("Deciphering Over: ");
        for (i = 0; i < LEN_8; i++)
            printf("%c", string[i]);
        printf("\n");
    }
    return string;
}
