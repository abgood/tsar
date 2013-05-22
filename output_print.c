#include "tsar.h"

/* 去除空格 */
char *trim(char *src, int max_len) {
    int cur_len = 0;
    char *index = src;

    while (*index == ' ' && cur_len < max_len) {
        index++;
        cur_len++;
    }
    return index;
}

/* 检查信息 */
void running_check(int check_type) {
    char filename[LEN_128] = {0};
    FILE *fp;
    int total_num = 0;
    char line[2][LEN_10240];
    double *st_array;

    sprintf(filename, "%s", conf.output_file_path);

    /* open log file */
    if (!(fp = fopen(filename, "r"))) {
        do_debug(LOG_FATAL, "unable to open the log file %s.\n", filename);
    }

    memset(&line[0], 0, LEN_10240);

    /* file end, 空文件 */
    if (fseek(fp , -1, SEEK_END) != 0) {
        do_debug(LOG_FATAL, "%s fseek error: %s\n", filename, strerror(errno));
    }

    /* 统计日志文件的行数 */
    while (1) {
        if (fgetc(fp) == '\n') {
            ++total_num;
        }
        if (total_num == 3) {
            break;
        }
        if (fseek(fp, -2, SEEK_CUR) != 0) {
            /* just 1 or 2 line, goto file header */
            if (fseek(fp, 0, SEEK_SET) != 0) {
                do_debug(LOG_FATAL, "%s fseek error:%s\n", filename, strerror(errno));
            }
            break;
        }
    }

    /* judge log file lines */
    if (total_num == 0) {           /* 日志文件0行 */
        printf("log file is no line\n");
    } else if (total_num == 1) {    /* 日志文件1行 */
        /* 第二行(line[1])来自于tsar.data里 */
        memset(&line[1], 0, LEN_10240);
        /* read one line from log file */
        if (!fgets(line[1], LEN_10240, fp)) {
            do_debug(LOG_FATAL, "fgets error: %s\n", strerror(errno));
        }
        /* close log file */
        if (fclose(fp) < 0) {
            do_debug(LOG_FATAL, "fclose error: %s\n", strerror(errno));
        }
        sprintf(filename, "%s.1", conf.output_file_path);
        if (!(fp = fopen(filename, "r"))) {
            do_debug(LOG_FATAL, "unable to open the log file %s.\n", filename);
        }
        total_num = 0;
        /* go to the start of the last line at tsar.data.1 */
        /* file end, 空文件 */
        if (fseek(fp , -1, SEEK_END) != 0) {
            do_debug(LOG_FATAL, "%s fseek error: %s\n", filename, strerror(errno));
        }

        /* 统计日志文件的行数 */
        while (1) {
            if (fgetc(fp) == '\n') {
                ++total_num;
            }
            if (total_num == 3) {
                break;
            }
            if (fseek(fp, -2, SEEK_CUR) != 0) {
                /* just 1 or 2 line, goto file header */
                if (fseek(fp, 0, SEEK_SET) != 0) {
                    do_debug(LOG_FATAL, "%s fseek error:%s\n", filename, strerror(errno));
                }
                break;
            }
        }

        if (total_num < 1) {
            do_debug(LOG_FATAL, "not enough lines at log file %s\n", filename);
        }
        /* 第一行(line[0])来自于tsar.data里 */
        memset(&line[0], 0, LEN_10240);
        if (!fgets(line[0], LEN_10240, fp)) {
            do_debug(LOG_FATAL, "fgets error: %s\n", strerror(errno));
        }
    } else {                        /* 日志文件多于1行 */
        /* first line */
        memset(&line[0], 0, LEN_10240);
        if (!fgets(line[0], LEN_10240, fp)) {
            do_debug(LOG_FATAL, "fgets error: %s\n", strerror(errno));
        }
        /* second line */
        memset(&line[1], 0, LEN_10240);
        if (!fgets(line[1], LEN_10240, fp)) {
            do_debug(LOG_FATAL, "fgets error: %s\n", strerror(errno));
        }
    }

    init_module_fields();

    read_line_to_module_record(line[0]);
    collect_record_stat();

    read_line_to_module_record(line[1]);
    collect_record_stat();

    /* display check detail */
    if (check_type == RUN_CHECK_NEW) {
        int i, j, k;
        struct module *mod;
        printf("%s\ttsar\t", conf.host_name);

        /* each module */
        for (i = 0; i < statis.total_mod_num; i++) {
            mod = &mods[i];
            if (!mod->enable) {
                continue;
            }

            struct mod_info *info = mod->info;
            /* get mod name */
            char *mod_name = strstr(mod->opt_line, "--");
            if (mod_name) {
                mod_name += 2;
            }

            char opt[LEN_128] = {0};
            char *n_record = strdup(mod->record);
            char *token = strtok(n_record, ITEM_SPLIT);
            char *s_token;

            /* each item */
            for (j = 0; j < mod->n_item; j++) {
                if (token) {
                    s_token = strstr(token, ITEM_SPSTART);
                    if (s_token) {
                        memset(opt, 0, LEN_128);
                        strncat(opt, token, s_token - token);
                        strcat(opt, ":");
                    }
                }

                st_array = &mod->st_array[j * mod->n_col];
                /* each col */
                for (k = 0; k < mod->n_col; k++) {
                    /* spec_xxx write in tsar.conf */
                    if (mod->spec) {
                        /* no data and no flag */
                        if (!st_array && !mod->st_flag) {
                            printf("No data\n");
                        } else {
                            if (((DATA_SUMMARY == conf.print_mode) && (SPEC_BIT == info[k].summary_bit)) 
                                    || ((DATA_DETAIL == conf.print_mode) && (SPEC_BIT == info[k].summary_bit))) {
                                printf("%s:%s%s=", mod_name, opt, trim(info[k].hdr, LEN_128));
                                printf("%0.1f ", st_array[k]);
                            }
                        }
                    } else {
                        /* no data and no flag */
                        if (!st_array && !mod->st_flag) {
                            printf("No data\n");
                        } else {
                            if (((DATA_SUMMARY == conf.print_mode) && (SUMMARY_BIT == info[k].summary_bit)) 
                                    || ((DATA_DETAIL == conf.print_mode) && (HIDE_BIT != info[k].summary_bit))) {
                                printf("%s:%s%s=", mod_name, opt, trim(info[k].hdr, LEN_128));
                                printf("%0.1f ", st_array[k]);
                            }
                        }
                    }
                }

                /* next item for module */
                if (token) {
                    token = strtok(NULL, ITEM_SPLIT);
                }
            }

            /* free mem */
            if (n_record) {
                free(n_record);
                n_record = NULL;
            }
        }
        printf("\n");
        if (fclose(fp) < 0) {
            do_debug(LOG_FATAL, "fclose error: %s", strerror(errno));
        }
        fp = NULL;
        return;
    }
}
