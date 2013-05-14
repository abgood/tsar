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
    char opt_line[LEN_32];      /* 模块opt信息 */
    char usage[LEN_256];        /* 模块usage信息 */
    char print_item[LEN_32];    /* 模块打印项 */

    void (*mod_register) (struct module *);     /* 模块管理函数 */
    void (*data_collect) (struct module *, char *);     /* 模块数据收集 */
    void (*set_st_record) (struct module *, double *, U_64 *, U_64 *, int);      /* 模块数据设置 */
};

/* 加载模块 */
void load_modules(void);

/* 注册模块区域,完善模块相关信息 */
void register_mod_fileds(struct module *, const char *, const char *,
        struct mod_info *, int, void *, void *);

#endif
