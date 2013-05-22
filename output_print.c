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

/* 定位从哪个tsar.data文件哪个位置开始打印
 * 0: OK
 * 1: 老文件
 * 2: 新文件
 * 3: 已是最新文件
 * 4: 查找的时间数据丢失
 * 5: 日志格式
 * 6: 其它错误
 * */
int find_offset_from_start(FILE *fp, int number) {
    long fset, fend, file_len, line_len, off_start, off_end, offset;
    char line[10240] = {0};
    time_t now, t_token, t_get;
    char *p_sec_token;

    /* file end */
    if (fseek(fp, 0, SEEK_END) != 0) {
        do_debug(LOG_FATAL, "fseek error: %s", strerror(errno));
    }
    if ((fend = ftell(fp)) < 0) {
        do_debug(LOG_FATAL, "ftell error: %s", strerror(errno));
    }
    /* file head */
    if (fseek(fp, 0, SEEK_SET) != 0) {
        do_debug(LOG_FATAL, "fseek error: %s", strerror(errno));
    }
    if ((fset = ftell(fp)) < 0) {
        do_debug(LOG_FATAL, "ftell error: %s", strerror(errno));
    }
    file_len = fend - fset;

    memset(&line, 0, LEN_10240);
    /* read one line */
    if (!fgets(line, LEN_10240, fp)) {
        do_debug(LOG_FATAL, "fgets error: %s", strerror(errno));
    }
    line_len = strlen(line);

    /* get now time */
    time(&now);

    /* 打印某一天的日志 */
    if (conf.print_day > 180) {
        printf("you will print %dth day (>180)\n", conf.print_day);
    }
    if (conf.print_day >= 0) {
        // printf("you will print %dth day (>=0)\n", conf.print_day);
        if (conf.print_day > 180) {
            conf.print_day = 180;
        }
        /* 当前时间的前8个小时 */
        now = now - now % (24 * 60 * 60) - (8 * 60 * 60);
        /* print_day那一天 */
        t_token = now - conf.print_day * (24 * 60 * 60) - (60 * conf.print_nline_interval);
        /* the start of print time */
        conf.print_start_time = t_token;
        /* end time, 24 hours */
        conf.print_end_time = t_token + (24 * 60 * 60) + (60 * conf.print_nline_interval);
    } else {
        // printf("you will print %dth day (<0)\n", conf.print_day);
        /* 某一天到现在的时间段 */
        if (conf.print_ndays > 180) {
            conf.print_ndays = 180;
        }
        /* now为间隔的整数倍 */
        now = now - now % (60 * conf.print_nline_interval);
        /* 过去时间点 */
        t_token = now - conf.print_ndays * (24 * 60 * 60) - (60 * conf.print_nline_interval);
        /* start time */
        conf.print_start_time = t_token;
        /* end time, current time */
        conf.print_end_time = now + (60 * conf.print_nline_interval);
    }

    offset = off_start = 0;
    off_end = file_len;
    while (1) {
        /* file middle */
        offset = (off_start + off_end) / 2;
        memset(&line, 0, LEN_10240);
        /* according to the offset position file */
        if (fseek(fp, offset, SEEK_SET) != 0) {
            do_debug(LOG_FATAL, "fseek error: %s", strerror(errno));
        }
        /* read one line */
        if (!fgets(line, LEN_10240, fp)) {
            do_debug(LOG_FATAL, "fgets error: %s", strerror(errno));
        }
        memset(&line, 0, LEN_10240);
        /* read one line again */
        if (!fgets(line, LEN_10240, fp)) {
            do_debug(LOG_FATAL, "fgets error: %s", strerror(errno));
        }
        if (0 != line[0] && offset > line_len) {
            p_sec_token = strstr(line, SECTION_SPLIT);
            if (p_sec_token) {
                /* get line time */
                *p_sec_token = '\0';
                t_get = atol(line);
                /* print start time and find time from file less than 60 seconds
                 * the file number is OK
                 */
                if (abs(t_get - t_token) <= 60) {
                    conf.print_file_number = number;
                    return 0;
                }
                /* get time is bigger
                 * get time later than print start time
                 * 往上
                 */
                if (t_get > t_token) {
                    off_end = offset;
                } else if (t_get < t_token) {
                    /* 往下 */
                    off_start = offset;
                }
            } else {
                /* log format error */
                return 5;
            }
        } else {        /* 时间开头为0, offset定位的文件长度小于文件一行长度 */
            /* 文件尾 */
            if (off_end == file_len) {
                if (number > 0) {
                    /* 下一个较新文件, number-1 */
                    conf.print_file_number = number - 1;
                    return 2;
                } else {    /* 已经是最新文件 */
                    return 3;
                }
            }
            /* 文件头 */
            if (off_start == 0) {
                /* 需要一个较老文件, number+1 */
                conf.print_file_number = number;
                return 1;
            }
            return 6;
        }

        if (offset == (off_start + off_end) / 2) {
            if (off_start != 0) {
                conf.print_file_number = number;
                return 4;
            }
            return 6;
        }
    }
}

void get_mod_hdr(char *hdr, const struct module *mod) {
    int i, pos = 0;
    struct mod_info *info = mod->info;

    for (i = 0; i < mod->n_col; i++) {
        if (mod->spec) {
        } else {
        }
    }
}

/* print header */
void print_header(void) {
    int i;
    struct module *mod;
    char opt_line[LEN_10240] = {0};
    char hdr_line[LEN_10240] = {0};
    char n_opt[LEN_256] = {0};
    char mod_hdr[LEN_256] = {0};

    if (conf.print_mode == RUN_PRINT_LIVE) {
        sprintf(opt_line, "Time             %s", PRINT_SEC_SPLIT);
        sprintf(hdr_line, "Time             %s", PRINT_SEC_SPLIT);
    } else {
        sprintf(opt_line, "Time          %s", PRINT_SEC_SPLIT);
        sprintf(hdr_line, "Time          %s", PRINT_SEC_SPLIT);
    }

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (!mod->enable) {
            continue;
        }

        memset(n_opt, 0, sizeof(n_opt));
        memset(mod_hdr, 0, sizeof(mod_hdr));
        get_mod_hdr(mod_hdr, mod);
    }
}

/* set record time */
long set_record_time(const char *line) {
    printf("%s\n", line);
}

/* 初始化打印界面 */
FILE *init_running_print(void) {
    FILE *fp, *fptmp;
    int i = 0, k = 0;
    char filename[LEN_128] = {0};
    char line[LEN_10240] = {0};

    /* 打印尾 */
    conf.print_tail = 1;

    if (!(fp = fopen(conf.output_file_path, "r"))) {
        do_debug(LOG_FATAL, "unable to open the log file %s\n", conf.output_file_path);
    }

    /* 根据此号来读日志文件 */
    conf.print_file_number = -1;

    /* k:错误号, i:文件号 */
    k = find_offset_from_start(fp, i);

    /* 1: 老文件 */
    if (k == 1) {
        for (i = 0; ; i++) {
            memset(filename, 0, LEN_128);
            sprintf(filename, "%s.%d", conf.output_file_path, i);
            /* 打开老点文件 */
            if (!(fptmp = fopen(filename, "r"))) {
                conf.print_file_number = i - 1;
                break;
            }

            k = find_offset_from_start(fptmp, i);
            /* OK */
            if (k == 0 || k == 4) {
                if (fclose(fp) < 0) {
                    do_debug(LOG_FATAL, "fclose error: %s\n", strerror(errno));
                }
                fp = fptmp;
                break;
            }

            /* new file */
            if (k == 2) {
                if (fseek(fp, 0, SEEK_SET) != 0) {
                    do_debug(LOG_FATAL, "fseek error: %s\n", strerror(errno));
                }
                if (fclose(fptmp) < 0) {
                    do_debug(LOG_FATAL, "fclose error: %s\n", strerror(errno));
                }
                break;
            }

            /* old file */
            if (k == 1) {
                if (fclose(fp) < 0) {
                    do_debug(LOG_FATAL, "fclose error: %s\n", strerror(errno));
                }
                fp = fptmp;
                continue;
            }

            /* else error */
            if (k == 5 || k == 6) {
                do_debug(LOG_FATAL, "log format error or find_offset_from_start have a bug. error code=%d\n", k);
            }
        }
    }

    if (k == 5 || k == 6) {
        do_debug(LOG_FATAL, "log format error or find_offset_from_start have a bug. error code=%d\n", k);
    }

    /* get record */
    if (!fgets(line, LEN_10240, fp)) {
        do_debug(LOG_FATAL, "can't get enough log info\n");
    }

    /* read one line to init module */
    read_line_to_module_record(line);

    /* print header */
    print_header();

    /* init module fields */
    init_module_fields();

    /* set record time */
    set_record_time(line);

    return fp;
}

/* 显示日志文件信息 */
void running_print(void) {
    FILE *fp;

    /* 初始化打印界面 */
    fp = init_running_print();
}
