/*
 * 解析tsar配置信息
 */

#include "tsar.h"

/* 解析tsar配置 */
void parse_config_file(const char *file_name) {
    FILE *fp;

    /* 打开配置文件 */
    if (!(fp = fopen(file_name, "r")))
        do_debug(LOG_FATAL, "Unable to open configuration file: %s", file_name);
}
