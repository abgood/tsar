#ifndef TSAR_DEFINE_H
#define TSAR_DEFINE_H

/* 相关文件 */
#define DEFAULT_CONF_FILE "tsar.conf"   /* tsar配置文件 */

/* 长度 */
#define LEN_32 32
#define LEN_56 64
#define LEN_128 128
#define LEN_256 256
#define LEN_512 512
#define LEN_1024 1024
#define LEN_4096 4096
#define LEN_10240 10240

/* 字符分割 */
#define SECTION_SPLIT "|"   /* 模块间分割符 */
#define ITEM_SPLIT ";"      /* 模块内项目分割符 */
#define W_SPACE " \t\r\n"   /* tsar配置文件分割符 */

/* 最大限制数 */
#define MAX_COL_NUM 64  /* 模块最大列数 */
#define MAX_MOD_NUM 32  /* 最多模块数 */

/* 模块各项状态 */
enum {
    HIDE_BIT,
    DETAIL_BIT,
    SUMMARY_BIT,
    SPEC_BIT
};

#endif
