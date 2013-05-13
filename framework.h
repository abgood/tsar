#ifndef TSAR_FRAMEWORK_H
#define TSAR_FRAMEWORK_H

#include "define.h"

struct mod_info {
    char hdr[LEN_128];      /* 模块小分类名 */
    int summary_bit;        /* 特殊位设置,详细,简要,隐藏 */
    int merge_mode;         /* 合并模式 */
    int stats_opt;          /* 状态 */
};

struct module {
    char name[LEN_32];          /* 模块名字 */
    char parameter[LEN_256];    /* 模块启动后的参数 */
    void *lib;                  /* 模块文件描述符 */
    int enable;                 /* 启用模块 */
    int spec;                   /* 启用特殊模块 */
    struct mod_info *info;      /* mod_info结构指针 */
    int n_col;                  /* mod的列数 */
};

void load_modules(void);

#endif
