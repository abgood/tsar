#ifndef TSAR_CONFIG_H
#define TSAR_CONFIG_H

#include "define.h"

struct configure {
    int debug_level;                    /* 日志级别 */
    char config_file[LEN_128];          /* tsar配置文件默认路径 */

    char output_interface[LEN_128];     /* 日志保存形式 */
    char output_file_path[LEN_128];     /* 日志保存文件路径 */
    char output_db_addr[LEN_512];       /* 日志DB地址 */
    char output_db_mod[LEN_512];        /* 输出到DB中的模块 */
    char output_nagios_mod[LEN_512];    /* 输出到nagios中的模块 */
    char output_stdio_mod[LEN_512];     /* 输出到stdio中的模块 */
    char output_print_mod[LEN_512];     /* argv中要打印的模块 */

    char server_addr[LEN_512];          /* nagios服务器ip地址 */
    int *server_port;                   /* nagios服务器端口 */
    int *cycle_time;                    /* 发送信息到nagios服务器的周期 */

    char send_nsca_cmd[LEN_512];        /* send_nsca命令路径 */
    char send_nsca_conf[LEN_512];       /* send_nsca配置路径 */

    int mod_num;                        /* threshold启用的模块数量 */
    char check_name[MAX_MOD_NUM][LEN_32];/* threshold里的模块名字 */
    float wmin[MAX_MOD_NUM];            /* threshold阀值1 */
    float wmax[MAX_MOD_NUM];            /* threshold阀值2 */
    float cmin[MAX_MOD_NUM];            /* threshold阀值3 */
    float cmax[MAX_MOD_NUM];            /* threshold阀值4 */

    int print_day;                      /* 当前时间前的哪一天需要打印 */
    int print_ndays;                    /* 当前时间前的哪一天到当前时间需要打印 */
    int running_mode;                   /* 运行模式 */
    int print_interval;                 /* 打印间隔秒 */
    int print_merge;                    /* 模块内的多项目合并模式 */
    int print_detail;                   /* 打印详细信息 */
    int print_tail;                     /* 打印尾 */
    int print_nline_interval;           /* 打印间隔行 */
    int print_mode;                     /* 打印模式, 简要or详细 */
};

/* 解析tsar.conf配置文件 */
void parse_config_file(const char *);

/* 解析tsar包含配置文件 */
void get_include_conf(void);

/* 设置模块特殊字段 */
void set_special_field(const char *);

/* 设置模块特殊项 */
void set_special_item(const char *);

#endif
