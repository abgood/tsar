#include "tsar.h"

/* mod->record输出到file */
void output_file(void) {
    int i, ret = 0;
    FILE *fp;
    char s_time[LEN_256] = {0};
    char line[LEN_10240] = {0};
    char detail[LEN_4096] = {0};
    struct module *mod = NULL;

    /* 可读写tsar.data文件 */
    if (!(fp = fopen(conf.output_file_path, "a+"))) {
        if (!(fp = fopen(conf.output_file_path, "w")))
            do_debug(LOG_FATAL, "output_file: can't create data file = %s  err=%d\n", conf.output_file_path, errno);
    }

    sprintf(s_time, "%ld", statis.cur_time);
    strcat(line, s_time);       /* 时间放入写入行 */

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (mod->enable && strlen(mod->record)) {
            /* 组合信息到detail */
            sprintf(detail, "%s%s%s%s", SECTION_SPLIT, mod->opt_line, STRING_SPLIT, mod->record);
            /* 收集到的信息放入line中 */
            strcat(line, detail);
            ret = 1;
        }
    }
    strcat(line, "\n");

    /* line写入文件 */
    if (ret) {
        if (fputs(line, fp) < 0)
            do_debug(LOG_ERR, "write line error\n");
    }

    if (fclose(fp) < 0)
        do_debug(LOG_FATAL, "fclose error:%s\n", strerror(errno));

    if (chmod(conf.output_file_path, 0666) < 0)
        do_debug(LOG_WARN, "chmod file %s error\n", conf.output_file_path);
}
