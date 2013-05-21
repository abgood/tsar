/*
 * Tsar主程序
 */

#include "tsar.h"

struct configure conf;
struct statistic statis;
struct module mods[MAX_MOD_NUM];

/* longopts结构 */
const struct option longopts[] = {
    { "cron", no_argument, NULL, 'c' },
    { "check", no_argument, NULL, 'C' },
    { "interval", required_argument, NULL, 'i' },
    { "list", no_argument, NULL, 'L' },
    { "live", no_argument, NULL, 'l' },
    { "file", required_argument, NULL, 'f' },
    { "ndays", required_argument, NULL, 'n' },
    { "date", required_argument, NULL, 'd' },
    { "merge", no_argument, NULL, 'm' },
    { "detail", no_argument, NULL, 'D' },
    { "spec", required_argument, NULL, 's' },
    { "item", required_argument, NULL, 'I' },
    { "encry", required_argument, NULL, 'e' },
    { "decry", required_argument, NULL, 'j' },
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0},
};

/* tsar说明信息 */
void usage(void) {
    int    i;
    struct module *mod;

    fprintf(stderr,
            "Usage: tsar [options]\n"
            "Options:\n"
            "    --check/-C     display last record for alert.example:tsar --check / tsar --check --cpu --io\n"
            "    --cron/-c      run in cron mode, output data to file\n"
            "    --interval/-i  specify intervals numbers, in minutes if with --live, it is in seconds\n"
            "    --list/-L      list enabled modules\n"
            "    --live/-l      running print live mode, which module will print\n"
            "    --file/-f      specify a filepath as input\n"
            "    --ndays/-n     show the value for the past days (default: 1)\n"
            "    --date/-d      show the value for the specify day(n or YYYYMMDD)\n"
            "    --merge/-m     merge multiply item to one\n"
            "    --detail/-D    do not conver data to K/M/G\n"
            "    --spec/-s      show spec field data, tsar --cpu -s sys,util\n"
            "    --item/-I      show spec item data, tsar --io -I sda\n"
            "    --encry/-e     encrypt the string, tsar -e <string> \n"
            "    --decry/-j     decrypt the string, tsar -j <string> \n"
            "    --help/-h      help\n");

    fprintf(stderr,
            "Modules Enabled:\n"
           );

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (mod->usage) {
            fprintf(stderr, "%s", mod->usage);
            fprintf(stderr, "\n");
        }
    }

    exit(0);
}

/* 参数解析 */
static void main_init(int argc, char **argv) {
    int opt, oind = 0;

    while ((opt = getopt_long(argc, argv, ":cCi:Llf:n:d:s:I:e:j:mhD", longopts, NULL)) != -1) {
        oind++;
        switch (opt) {
            case 'c':       /* 计划任务 */
                conf.running_mode = RUN_CRON;
                break;
            case 'C':       /* 检查 */
                conf.running_mode = RUN_CHECK_NEW;
                break;
            case 'l':       /* 实时 */
                conf.running_mode = RUN_PRINT_LIVE;
                break;
            case 'L':       /* 模块列表 */
                conf.running_mode = RUN_LIST;
                break;
            case 'i':       /* 打印间隔秒 */
                conf.print_interval = atoi(optarg);
                oind++;
                break;
            case 'f':       /* 输出文件 */
                strcpy(conf.output_file_path, optarg);
                break;
            case 's':       /* 显示模块特殊字段 */
                set_special_field(optarg);
                break;
            case 'I':       /* 设置模块特殊项 */
                set_special_item(optarg);
                break;
            case 'n':       /* 几前天到当前时间 */
                conf.print_ndays = atoi(optarg);
                oind++;
                break;
            case 'd':       /* 几天前的那一天 */
                conf.print_day = atoi(optarg);
                oind++;
                break;
            case 'm':       /* 模块内多项目合并模式 */
                conf.print_merge = MERGE_ITEM;
                break;
            case 'D':       /* conver data to K/M/G */
                conf.print_detail = TRUE;
                break;
            case 'e':       /* 加密字符串 */
                enc_code(optarg);
                break;
            case 'j':       /* 解密字符串 */
                dec_code(optarg, 1);
                break;
            case 'h':       /* tsar帮助 */
                usage();
            case ':':       /* 选项后缺少参数 */
                printf("must have parameter\n");
                usage();
            case '?':       /* 命令行未定义选项参数 */
                if (argv[oind] && strstr(argv[oind], "--")) {
                    strcat(conf.output_print_mod, argv[oind]);
                    strcat(conf.output_print_mod, DATA_SPLIT);
                } else
                    usage();
        }
    }
/* 设置默认参数 */
    /* not -n, 没有多少天来 */
    if (!conf.print_ndays)
        conf.print_ndays = 1;

    /* not -i, 没有打印间隔秒 */
    if (!conf.print_interval)
        conf.print_interval = DEFAULT_PRINT_INTERVAL;

    /* 没有运行模式则RUN_PRINT */
    if (RUN_NULL == conf.running_mode) 
        conf.running_mode = RUN_PRINT;

    /* -C, 检查模式 */
    if (conf.running_mode == RUN_CHECK_NEW) {
        conf.print_interval = 60;
        conf.print_tail = 0;
        conf.print_nline_interval = conf.print_interval;    /* 打印间隔行 */
    }

    /* 打印模式, 简要or详细 */
    if (!strlen(conf.output_print_mod))
        conf.print_mode = DATA_SUMMARY;
    else
        conf.print_mode = DATA_DETAIL;

    /* 默认配置文件 tsar.conf */
    strcpy(conf.config_file, DEFAULT_CONF_FILE_PATH);
    if (access(conf.config_file, F_OK))
        do_debug(LOG_FATAL, "main_init: can't find tsar.conf\n");
}

/* 显示启用模块列表 */
void running_list(void) {
    int i;
    struct module *mod;

    printf("tsar enable follow modules:\n");

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        printf("\t%s\n", mod->name + 4);
    }
}

/* 计划任务运行 */
void running_cron(void) {
    /* 调用模块,收集记录 */
    collect_record();

    /* 输出到file */
    if (strstr(conf.output_interface, "file"))
        output_file();

    /* 输出到db */
    if (strstr(conf.output_interface, "db"))
        output_db();

    /* 输出到nagios */
    if (strstr(conf.output_interface, "nagios"))
        output_nagios();
}

int main (int argc, char **argv) {
    /* 解析tsar配置文件 */
    parse_config_file(DEFAULT_CONF_FILE);

    /* 加载模块 */
    load_modules();

    /* 保存当前时间 */
    statis.cur_time = time(NULL);

    /* 当前时间前的哪一天需要打印 */
    conf.print_day = -1;

    /* 解析参数 */
    main_init(argc, argv);

    /* 开始执行 */
    switch (conf.running_mode) {
        case RUN_LIST:          /* -L */
            running_list();
            break;
        case RUN_CRON:          /* -c */
            conf.print_mode = DATA_DETAIL;
            running_cron();
            break;
    }

    return 0;
}
