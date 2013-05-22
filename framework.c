#include "tsar.h"

/* 从模块目录里加载模块 */
void load_modules(void) {
    int i;
    char buff[LEN_128] = {0};
    char mod_path[LEN_128] = {0};
    struct module *mod;
    int (*mod_register)(struct module *);

    /* in config.h conf.module_path no data */
    if (strlen(conf.module_path) == 0) {
        sprintf(buff, "/root/code/tsar/modules");
    } else {
        sprintf(buff, conf.module_path);
    }
    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (!mod->lib) {
            sprintf(mod_path, "%s/%s.so", buff, mod->name);
            if (!(mod->lib = dlopen(mod_path, RTLD_NOW|RTLD_GLOBAL)))
                do_debug(LOG_ERR, "load_modules: dlopen module %s err %s\n", mod->name, dlerror());
            else{
                mod_register = dlsym(mod->lib, "mod_register");
                if (dlerror()) {
                    do_debug(LOG_ERR, "load_modules: dlsym module %s err %s\n", mod->name, dlerror());
                    break;
                } else {
                    mod_register(mod);
                    mod->enable = 1;
                    mod->spec = 0;
                    do_debug(LOG_INFO, "load_modules: load new module '%s' to mods\n", mod_path);
                }
            }
        }
    }
}

/* 注册模块区域,完善模块相关信息 */
void register_mod_fileds(struct module *mod, const char *opt, const char *usage,
        struct mod_info *info, int n_col, void *data_collect, void *set_st_record) {
    sprintf(mod->opt_line, "%s", opt);
    sprintf(mod->usage, "%s", usage);
    mod->info = info;
    mod->n_col = n_col;
    mod->data_collect = data_collect;
    mod->set_st_record = set_st_record;
}

/* 设置module收集到的信息到mod->record */
void set_mod_record(struct module *mod, const char *record) {
    if (record)
        sprintf(mod->record, "%s", record);
}

/* tsar运行开始先收集数据再输出 */
void collect_record(void) {
    int i;
    struct module *mod = NULL;

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (!mod->enable)
            continue;

        memset(mod->record, 0, sizeof(mod->record));
        if (mod->data_collect)
            mod->data_collect(mod, mod->parameter);
    }
}

/* 循环第一个字串, 是否包含第二个字串, 有则返回1 */
int is_include_string(const char *mods, const char *mod) {
    char *token, n_str[LEN_512] = {0};

    /* 第一字串拷到n_str里 */
    memcpy(n_str, mods, strlen(mods));

    /* 第一字串里的每一项都要与第二字串比较 */
    token = strtok(n_str, DATA_SPLIT);
    while (token) {
        if (!strcmp(token, mod))
            return 1;
        token = strtok(NULL, DATA_SPLIT);
    }
    return 0;
}

/* 重新加载输出设置模块
 *  如果不在mods里, disable此模块
 */
int reload_modules(const char *s_mod) {
    int i;
    int reload = 0;
    struct module *mod;

    if (!s_mod || !strlen(s_mod))
        return reload;

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (is_include_string(s_mod, mod->name) || is_include_string(s_mod, mod->opt_line)) {
            mod->enable = 1;
            reload = 1;
        } else
            mod->enable = 0;
    }
    return reload;
}

/* 把一条记录保存至mod->record内 */
void read_line_to_module_record(char *line) {
    int i;
    char *s_token, *e_token;
    struct module *mod;

    line[strlen(line) - 1] = '\0';

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (mod->enable) {
            char mod_opt[LEN_64];
            /* 保存每个模块匹配的子字符串 */
            sprintf(mod_opt, "%s%s%s", SECTION_SPLIT, mod->opt_line, STRING_SPLIT);
            memset(mod->record, 0, sizeof(mod->record));

            if (!(s_token = strstr(line, mod_opt)))
                continue;

            /* 每个模块所需信息的开头 */
            s_token += sizeof(SECTION_SPLIT) + strlen(mod->opt_line) + sizeof(STRING_SPLIT) - 2;
            /* 每个模块所需信息的结尾 */
            e_token = strstr(s_token, SECTION_SPLIT);

            if (e_token)
                memcpy(mod->record, s_token, e_token - s_token);
            else
                /* strlen(line) - (s_token - line) == 总长度 - (匹配地址 - 开头地址) ===> 剩余长度 */
                memcpy(mod->record, s_token, strlen(line) - (s_token - line));
        }
    }
}

/* 开辟相关模块内存空间并初始化 */
void init_module_fields(void) {
    int i;
    struct module *mod;

    /* 根据每个模块分配内存空间 */
    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (!mod->enable)
            continue;

        /* 模式判断 */
        if (MERGE_ITEM == conf.print_merge)
            mod->n_item = 1;
        else
            /* 获得模块项目数 */
            mod->n_item = get_strtok_num(mod->record, ITEM_SPLIT);

        /* 分配内存空间 */
        if (mod->n_item) {
            mod->pre_array = (U_64 *)calloc(mod->n_item * mod->n_col, sizeof(U_64));
            mod->cur_array = (U_64 *)calloc(mod->n_item * mod->n_col, sizeof(U_64));
            mod->st_array = (double *)calloc(mod->n_item * mod->n_col, sizeof(double));
            /* 最后面的为总结部分内存空间 */
            if (conf.print_tail) {
                mod->max_array = (double *)calloc(mod->n_item * mod->n_col, sizeof(double));
                mod->mean_array = (double *)calloc(mod->n_item * mod->n_col, sizeof(double));
                mod->mean_array = (double *)calloc(mod->n_item * mod->n_col, sizeof(double));
            }
        }
    }
}

/* 模块项目数增加,重新分配模块占用内存空间 */
void realloc_module_array(struct module *mod, int n_n_item) {
    if (n_n_item > mod->n_item) {
        /* mod->pre_array之前已使用过 */
        if (mod->pre_array) {
            mod->pre_array = (U_64 *)realloc(mod->pre_array, n_n_item * mod->n_col * sizeof(U_64));
            mod->cur_array = (U_64 *)realloc(mod->cur_array, n_n_item * mod->n_col * sizeof(U_64));
            mod->st_array = (double *)realloc(mod->st_array, n_n_item * mod->n_col * sizeof(double));
            if (conf.print_tail) {
                mod->max_array = (double *)realloc(mod->max_array, n_n_item * mod->n_col * sizeof(double));
                mod->mean_array = (double *)realloc(mod->mean_array, n_n_item * mod->n_col * sizeof(double));
                mod->min_array = (double *)realloc(mod->min_array, n_n_item * mod->n_col * sizeof(double));
            }
        } else {
            mod->pre_array = (U_64 *)calloc(n_n_item * mod->n_col, sizeof(U_64));
            mod->cur_array = (U_64 *)calloc(n_n_item * mod->n_col, sizeof(U_64));
            mod->st_array = (double *)calloc(n_n_item * mod->n_col, sizeof(double));
            /* 最后面的为总结部分内存空间 */
            if (conf.print_tail) {
                mod->max_array = (double *)calloc(n_n_item * mod->n_col, sizeof(double));
                mod->mean_array = (double *)calloc(n_n_item * mod->n_col, sizeof(double));
                mod->mean_array = (double *)calloc(n_n_item * mod->n_col, sizeof(double));
            }
        }
    }
}

/* 把cur_array与pre_array设置到st_array */
void set_st_record(struct module *mod) {
    int i, j, k = 0;
    // struct mod_info *info = mod->info;
    mod->st_flag = 1;

    /* 模块每项 */
    for (i = 0; i < mod->n_item; i++) {
        /* 模块自己进行数据处理并保存至st_array */
        if (mod->set_st_record) {
            /* 把pre_array, cur_array, st_array传入函数, 不在用mod指向 */
            mod->set_st_record(mod, &mod->st_array[i * mod->n_col],
                    &mod->pre_array[i * mod->n_col],
                    &mod->cur_array[i * mod->n_col],
                    conf.print_interval);
        }

        /* 模块每列 */
        for (j = 0; j < mod->n_col; j++) {
            /* 没有set record */
            if (!mod->set_st_record) {
                printf("have not set_st_record\n");
            }

            /* 打印每列的尾部 */
            if (conf.print_tail) {
                printf("print tail\n");
            }

            k++;
        }
    }

    mod->n_record++;
}

/* 通过mod->record得出st_array内的数据 */
int collect_record_stat(void) {
    int no_p_hdr = 1;
    int i, ret, n_item;
    struct module *mod;
    U_64 *tmp;

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (!mod->enable)
            continue;

        ret = 0;
        /* st_array状态为0 */
        mod->st_flag = 0;

        n_item = get_strtok_num(mod->record, ITEM_SPLIT);

        if ((n_item = get_strtok_num(mod->record, ITEM_SPLIT))) {
            /* 非合并模式, 模块刚计算的项目数与模块之前记录的项目数是否相等, 重新分配模块占用内存空间 */
            if (MERGE_ITEM != conf.print_merge && n_item && n_item != mod->n_item) {
                no_p_hdr = 0;
                /* 重新分配模块占用内存空间 */
                realloc_module_array(mod, n_item);
            }
            /* mod->n_item重新赋值 */
            mod->n_item = n_item;

            /* 多项目为真 */
            if (strstr(mod->record, ITEM_SPLIT)) {
                // conf.print_merge = MERGE_NOT;
                /* 合并模式 */
                if (MERGE_ITEM == conf.print_merge) {
                    mod->n_item = 1;
                    /* 合并模块多项目的记录到cur_array里 */
                    ret = merge_mult_item_to_array(mod->cur_array, mod);
                } else {    /* 非合并模式 */
                    char item[LEN_128] = {0};
                    int num = 0;
                    int pos = 0;

                    /* 非合并模式下, 循环模块每项 
                     * 把每项记录以数组的方式根据num的值来存储
                     * 相当于一个二维数组, num控制行, n_col控制列
                     */
                    while (strtok_next_item(item, mod->record, &pos)) {
                        if (!(ret = convert_record_to_array(&mod->cur_array[num * mod->n_col], mod->n_col, item)))
                            break;
                        memset(item, 0, sizeof(item));
                        num++;
                    }
                }
            } else      /* 单项目 */
                /* mod->record ===> cur_array, ret为项目里的记录数 */
                ret = convert_record_to_array(mod->cur_array, mod->n_col, mod->record);

            /* 设置st_array里的记录
             * 第一次读记录时并未设置pre_flag状态
             * 再次读记录时则设置pre_flag状态 
             * pre_flag标识之前数据已被读
             * 即pre_array保存的有历史数据
             */
            if (no_p_hdr && mod->pre_flag && ret)
                set_st_record(mod);

            /* 设置pre_flag状态 */
            if (!ret)
                mod->pre_flag = 0;
            else
                mod->pre_flag = 1;
        } else 
            mod->pre_flag = 0;

        /* 交换cur_array与pre_array
         * 把当前处理得到的数据放入历史数据
         */
        tmp = mod->pre_array;
        mod->pre_array = mod->cur_array;
        mod->cur_array = tmp;
    }

    return no_p_hdr;
}

/* 取消列数为0的模块
 * 没有DETAIL_BIT状态, 此函数无用
 */ 
void disable_col_zero(void) {
    int i, j;
    struct module *mod;

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (!mod->enable)
            continue;

        if (!mod->n_col)
            mod->enable = 0;
        else {
            int p_col = 0;
            struct mod_info *info = mod->info;
            
            for (j = 0; j < mod->n_col; j++) {
                /* 匹配项 */
                // if (((DATA_SUMMARY == conf.print_mode) && (SUMMARY_BIT == info[j].summary_bit))
                if (((DATA_SUMMARY == conf.print_mode) && (HIDE_BIT != info[j].summary_bit))
                        || ((DATA_DETAIL == conf.print_mode) && (HIDE_BIT != info[j].summary_bit))) {
                    p_col++;
                    break;
                }
            }

            /* 未匹配到项 */
            if (!p_col)
                mod->enable = 0;
        }
    }
}
