/*
 * Tsar主程序
 */

#include "tsar.h"

struct configure conf;

int
main (int argc, char **argv) {
    /* 解析tsar配置文件 */
    parse_config_file(DEFAULT_CONF_FILE);

    return 0;
}
