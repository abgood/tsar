#include "tsar.h"

/* 从文件里读记录并保存至st_array */
int get_st_array_from_file(void) {
    char line[LEN_10240] = {0};
    char detail[LEN_1024] = {0};
    int i, ret = 0;
    struct module *mod = NULL;
    FILE *fp;
    char pre_line[10240] = {0};
    char *s_token;
    char pre_time[LEN_32] = {0};

    /* 合并模式 */
    conf.print_merge = MERGE_ITEM;
    sprintf(line, "%ld", statis.cur_time);
    /* mod->record整理到line里 */
    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (mod->enable && strlen(mod->record)) {
            memset(detail, 0, sizeof(detail));
            sprintf(detail, "%s%s%s%s", SECTION_SPLIT, mod->opt_line, STRING_SPLIT, mod->record);
            strcat(line, detail);
        }
    }

    if (strlen(line))
        strcat(line, "\n");

    /* 打开之前的记录文件,读取内容保存至pre_record */
    if ((fp = fopen(PRE_RECORD_FILE, "r"))) {
        if (!fgets(pre_line, LEN_10240, fp)) {
            if (fclose(fp) < 0)
                do_debug(LOG_FATAL, "fclose error: %s\n", strerror(errno));
            ret = -1;
            goto out;
        }
    } else {
        ret = -1;
        goto out;
    }

    /* 处理读取到的记录pre_line */
    s_token = strstr(pre_line, SECTION_SPLIT);
    if (!s_token) {
        ret = -1;
        goto out;
    }
    /* 获取.tsar.tmp文件记录的时间 */
    memcpy(pre_time, pre_line, s_token - pre_line);
    /* 当前时间与过去记录时间相等, goto out */
    if (!(conf.print_interval = statis.cur_time - atol(pre_time)))
        goto out;

    /* 把从.tsar.tmp文件里读出的pre_line保存至mod->record内 */
    read_line_to_module_record(pre_line);
    /* 开辟相关模块内存空间并初始化 */
    init_module_fields();       /* 此处初始化的内存空间为合并模式下 */
    /* 通过mod->record得出pre_array内的数据 */
    collect_record_stat();

    /* 模块当前收集到的数据保存在line里, 把line里数据再放回mod->record里 */
    read_line_to_module_record(line);
    /* cur_array == pre_line, pre_array == line */
    collect_record_stat();

out:
    /* mod->record ==> .tsar.tmp */
    if ((fp = fopen(PRE_RECORD_FILE, "w"))) {
        strcat(line, "\n");
        /* 写错误 */
        if (fputs(line, fp) < 0)
            do_debug(LOG_ERR, "fputs error:%s", strerror(errno));
        /* 关闭错误 */
        if (fclose(fp) < 0)
            do_debug(LOG_FATAL, "fclose error:%s", strerror(errno));
        chmod(PRE_RECORD_FILE, 0666);
    }

    return ret;
}

/* 获得模块项目数 */
int get_strtok_num(const char *str, const char *split) {
    /* 模块内至少有一项 */
    int num = 0;
    char *token, n_str[LEN_4096] = {0};

    if (!str || !strlen(str))
        return 0;

    memcpy(n_str, str, strlen(str));
    /* 模块里有多少个分号就有多少个项 */
    token = strtok(n_str, split);
    while (token) {
        num++;
        token = strtok(NULL, split);
    }
    return num;
}

/* 判断是否是数字 */
int is_digit(char *str) {
    while (*str) {
        if (!isdigit(*str++))
            return 0;
    }
    return 1;
}

/* 把mod->record记录转存至array数组里 */
int convert_record_to_array(U_64 *array, int l_array, const char *record) {
    int i = 0;
    char *token;
    char n_str[LEN_4096] = {0};

    if (!record || !strlen(record))
        return 0;
    memcpy(n_str, record, strlen(record));

    token = strtok(n_str, DATA_SPLIT);
    while (token) {
        /* 是数字且i < 列数 - 1, 要保存数组下标从0开始 */
        if (is_digit(token) && i < l_array)
            array[i++] = strtoull(token, NULL, 10);
        else
            return i;
        token = strtok(NULL, DATA_SPLIT);
    }
    return i;
}

/* 得到模块里每个项的记录 */
int strtok_next_item(char item[], char *record, int *start) {
    char *s_token, *e_token, *n_record;

    if (!record || !strlen(record) || strlen(record) <= *start)
        return 0;

    n_record = record + *start;
   /* 每个项的记录结尾, 分号, 子字符串 */
    if (!(e_token = strstr(n_record, ITEM_SPLIT)))
        return 0;
    /* 每个项的记录开始, 等号, 子字符串 */
    if (!(s_token = strstr(n_record, ITEM_SPSTART)))
        return 0;

    /* 把一项记录拷贝到item数组里 */
    memcpy(item, s_token + sizeof(ITEM_SPSTART) - 1, e_token - s_token - 1);
    /* 设置下一项的开始位移 */
    *start = e_token - record + sizeof(ITEM_SPLIT);
    return 1;
}

/* 模块里多个项合并到array数组里 */
int merge_one_string(U_64 *array, int l_array, char *string, struct module *mod, int n_item) {
    int i, len;
    U_64 array_2[MAX_MOD_NUM] = {0};
    struct mod_info *info = mod->info;

    /* 把string内的字符串转换为数字保存至array_2内 */
    if (!(len = convert_record_to_array(array_2, l_array, string)))
        return 0;

    /* eg. mod_partition, len == 3, i取值0,1,2 info[i].merge_mode均为MERGE_SUM, i == 3时为MERGE_AVG */
    for (i = 0; i < len; i++) {
        switch (info[i].merge_mode) {
            /* 求和 */
            case MERGE_SUM:
                array[i] += array_2[i];
                break;
            /* 求平均值 */
            case MERGE_AVG:
                array[i] = (array[i] * (n_item - 1) + array_2[i]) / n_item;
                break;
            default:
                break;
        }
    }
    return 1;
}

/* 合并模块多项目的记录到array数组里 */
int merge_mult_item_to_array(U_64 *array, struct module *mod) {
    int pos = 0;
    int n_item = 1;
    char item[LEN_128] = {0};

    memset(array, 0, sizeof(U_64) * mod->n_col);
    /* 循环取模块里每个项的记录 */
    while (strtok_next_item(item, mod->record, &pos)) {
        if (!merge_one_string(array, mod->n_col, item, mod, n_item))
            return 0;
        n_item++;
        memset(&item, 0, sizeof(item));
    }
    return 1;
}
