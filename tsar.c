/*
 * tsar主程序
 */

#include "tsar.h"

int main (int argc, char **argv) {
    /* 解析tsar配置 */
    parse_config_file(DEFAULT_CONF_FILE_PATH);

    return 0;
}
