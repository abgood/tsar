#ifndef TSAR_CONFIG_H
#define TSAR_CONFIG_H

struct configure {
    int debug_level;    /* 日志级别 */
};

void parse_config_file(const char *);

#endif
