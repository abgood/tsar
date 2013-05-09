/*
 * 解析tsar配置文件
 */

#include "tsar.h"

/* 解析mod */
static void
parse_mod(const char *mod_name) {
    printf("%s\n", mod_name);
}

/* 解析有效行 */
static int
parse_line(char *buff) {
    char *token;

    if (!(token = strtok(buff, W_SPACE)))
        (void) 0;
    else if (strstr(buff, "mod_"))
        parse_mod(token);
    else
        return 0;

    return 1;
}

void
parse_config_file(const char *file_name) {
    FILE *fp;
    char config_input_line[LEN_1024] = {0};
    char *token;

    /* 打开tsar配置文件 */
    if (!(fp = fopen(file_name, "r")))
        do_debug(LOG_FATAL, "Unable open file:%s\n", file_name);

    memset(&conf, '0', sizeof(conf));
    conf.debug_level = LOG_ERR;

    while (fgets(config_input_line, LEN_1024, fp)) {
        if ((token = strchr(config_input_line, '\n')))  /* 回车 */
            *token = '\0';
        if ((token = strchr(config_input_line, '\r')))  /* 换行 */
            *token = '\0';
        if (config_input_line[0] == '#') {              /* 以#开头 */
            memset(config_input_line, '0', LEN_1024);
            continue;
        }
        if (config_input_line[0] == '\0')                /* 空行 */
            continue;
        if (!parse_line(config_input_line))             /* 解析其它行 */
            do_debug(LOG_INFO, "parse_config_file: unknown key in '%s'\n", config_input_line);
    }

    /* 关闭文件 */
    if (fclose(fp) < 0)
        do_debug(LOG_FATAL, "fclose error: %s", strerror(errno));
}
