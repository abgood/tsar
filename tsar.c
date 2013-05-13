/*
 * Tsar主程序
 */

#include "tsar.h"

struct configure conf;
struct statistic statis;
struct module mods[MAX_MOD_NUM];

int main (int argc, char **argv) {
    /* 解析tsar配置文件 */
    parse_config_file(DEFAULT_CONF_FILE);

    /* 加载模块 */
    load_modules();

    printf("%d\n", statis.total_mod_num);

    return 0;
}
