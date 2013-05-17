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
    char opt_line[LEN_32];      /* 模块opt信息 */
    char usage[LEN_256];        /* 模块usage信息 */
    char print_item[LEN_32];    /* 模块打印项 */
    char record[LEN_4096];      /* 模块收集到的信息 */

    void *lib;                  /* 模块文件描述符 */
    int enable;                 /* 启用模块 */
    int spec;                   /* 启用特殊模块 */
    int n_col;                  /* mod的列数 */
    int n_item;                 /* 模块项目数 */

    U_64 *pre_array;            /* 以前记录信息内存空间 */
    U_64 *cur_array;            /* 当前记录信息内存空间 */
    double *st_array;           /* 处理后的信息内存空间 */
    double *max_array;          /* 信息里的各项最大值内存空间 */
    double *mean_array;         /* 信息里的各项中间值内存空间 */
    double *min_array;          /* 信息里的各项最小值内存空间 */

    int pre_flag:4;             /* pre_array的状态 */
    int st_flag:4;              /* st_array的状态 */

    struct mod_info *info;      /* mod_info结构指针 */

    void (*mod_register) (struct module *);     /* 模块管理函数 */
    void (*data_collect) (struct module *, char *);     /* 模块数据收集 */
    void (*set_st_record) (struct module *, double *, U_64 *, U_64 *, int);      /* 模块数据设置 */
};

/* 加载模块 */
void load_modules(void);

/* 注册模块区域,完善模块相关信息 */
void register_mod_fileds(struct module *, const char *, const char *,
        struct mod_info *, int, void *, void *);

/* tsar运行开始先收集数据再输出 */
void collect_record(void);

/* 设置module收集的信息到mod->record */
void set_mod_record(struct module *, const char *);

/* 重新加载输出设置模块, 不在mods里则disable此模块 */
int reload_modules(const char *);

/* 把一条记录保存至mod->record内 */
void read_line_to_module_record(char *);

/* 开辟相关模块内存空间并初始化 */
void init_module_fields(void);

/* 通过mod->record得出st_array内的数据 */
int collect_record_stat(void);

#endif
