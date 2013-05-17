#ifndef TSAR_COMMON_H
#define TSAR_COMMON_H

#define PRE_RECORD_FILE ".tsar.tmp"

#include "define.h"
#include "framework.h"

/* 从文件里读记录并保存至st_array */
int get_st_array_from_file(void);

/* 获得模块项目数 */
int get_strtok_num(const char *, const char *);

/* 把mod->record记录转存至cur_array里 */
int convert_record_to_array(U_64 *, int, const char *);

/* 合并模块多项目的记录到array数组里 */
int merge_mult_item_to_array(U_64 *, struct module *);

/* 得到模块里每个项的记录 */
int strtok_next_item(char *, char *, int *);

#endif
