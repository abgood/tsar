/*
 * 解析tsar配置文件
 */

#include "tsar.h"

/* 解析mod */
void parse_mod(const char *mod_name) {
    int i;
    /* 循环比较mod数组里面是否已包含mod_name */
    for (i = 0; i < statis.total_mod_num; i++) {
        struct module *mod = &mods[i];
        if (!strcmp(mod_name, mod->name))
            return;
    }
    struct module *mod = &mods[statis.total_mod_num++];
    char *token = strtok(NULL, W_SPACE);
    /* mod状态为on或者enable的则添加到mod数组 */
    if (token && (!strcasecmp(token, "on") || !strcasecmp(token, "enable"))) {
        strncpy(mod->name, mod_name, strlen(mod_name));
        token = strtok(NULL, W_SPACE);
        if (token)
            strncpy(mod->parameter, token, strlen(token));
        return;
    } else {
        memset(mod, 0, sizeof(struct module));
        statis.total_mod_num--;
    }
}

/* 特殊mod */
void special_mod(const char *spec_mod) {
    char mod_name[LEN_32] = {0};
    int i, j;
    struct module *mod;

    sprintf(mod_name, "mod_%s", spec_mod + 5);
    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (!strcmp(mod->name, mod_name)) {
            /* 加载特殊模块 */
            load_modules();
            char *token = strtok(NULL, W_SPACE);
            struct mod_info *info = mod->info;
            for (j = 0; j < mod->n_col; j++) {
                char *p = info[j].hdr;
                while (*p == ' ') p++;
                if (strstr(token, p)) {
                    info[j].summary_bit = SPEC_BIT;
                    mod->spec = 1;
                }
            }
        }
    }
}

/* 设置模块特殊字段 */
void set_special_field(const char *s) {
    int i, j;
    struct module *mod = NULL;

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        struct mod_info *info = mod->info;
        for (j = 0; j < mod->n_col; j++) {
            char *p = info[j].hdr;
            while (*p == ' ') p++;
            if (strstr(s, p)) {
                info[j].summary_bit = SPEC_BIT;
                mod->spec = 1;
            }
        }
    }
}

/* 设置模块特殊项 */
void set_special_item(const char *s) {
    int i;
    struct module *mod = NULL;

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        strcpy(mod->print_item, s);
    }
}

/* 解析字符串 */
void parse_string(char *var) {
    char *token = strtok(NULL, W_SPACE);
    if (token)
        strncpy(var, token, strlen(token));
}

/* 解析db_mod并追加 */
void parse_add_string(char *var) {
    char *token = strtok(NULL, W_SPACE);
    
    if (token) {
        strcat(token, ",");
        strncat(token, var, strlen(var));
    }
    if (token)
        strncpy(var, token, strlen(token));
}

/* 设置日志等级 */
void set_debug_level(void) {
    char *token = strtok(NULL, W_SPACE);
    if (token) {
        if (!strcmp(token, "INFO"))
            conf.debug_level = LOG_INFO;
        else if (!strcmp(token, "WARN"))
            conf.debug_level = LOG_WARN;
        else if (!strcmp(token, "DEBUG"))
            conf.debug_level = LOG_DEBUG;
        else if (!strcmp(token, "ERROR"))
            conf.debug_level = LOG_ERR;
        else if (!strcmp(token, "FATAL"))
            conf.debug_level = LOG_FATAL;
        else
            conf.debug_level = LOG_ERR;
    }
}

/* 解析整型 */
void parse_int(int *var) {
    char *token = strtok(NULL, W_SPACE);
    if (token == NULL)
        do_debug(LOG_FATAL, "Bungled line\n");
    *var = strtol(token, NULL, 0);
}

/* nagios界限值 */
void get_threshold(void) {
    char *token = strtok(NULL, W_SPACE);
    char tmp[4][LEN_32];

    if (conf.mod_num >= MAX_COL_NUM)
        do_debug(LOG_FATAL, "Too many mod threshold\n");

    sscanf(token, "%[^;];%[.N0-9];%[.N0-9];%[.N0-9];%[.N0-9];", conf.check_name[conf.mod_num], tmp[0], tmp[1], tmp[2], tmp[3]);
    /* Warnning最小值 */
    if (!strcmp(tmp[0], "N"))
        conf.wmin[conf.mod_num] = 0;
    else
        conf.wmin[conf.mod_num] = atof(tmp[0]);
    /* Warnning最大值 */
    if (!strcmp(tmp[1], "N"))
        conf.wmax[conf.mod_num] = 0;
    else
        conf.wmax[conf.mod_num] = atof(tmp[1]);
    /* Critical最小值 */
    if (!strcmp(tmp[2], "N"))
        conf.cmin[conf.mod_num] = 0;
    else
        conf.cmin[conf.mod_num] = atof(tmp[2]);
    /* Critical最大值 */
    if (!strcmp(tmp[3], "N"))
        conf.cmax[conf.mod_num] = 0;
    else
        conf.cmax[conf.mod_num] = atof(tmp[3]);
    conf.mod_num++;
}

/* 获取本机主机名和ip地址 */
void get_host(void) {
    struct ifaddrs *if_addr_struct;
    void *tmp = NULL;

    /* 主机名 */
    if (gethostname(conf.host_name, sizeof(conf.host_name)) != 0)
        do_debug(LOG_FATAL, "get host name error!\n");

    /* 主机ip */
    if (getifaddrs(&if_addr_struct) == -1)
        do_debug(LOG_FATAL, "get interface address struct error!\n");
    /* 循环网卡地址, 链表结构 */
    while (if_addr_struct != NULL) {
        /* eth0网卡, ipv4 */
        if (!strcmp(if_addr_struct->ifa_name, "eth0") && (if_addr_struct->ifa_addr->sa_family == AF_INET)) {
            /* 获取in_addr结构地址 */
            tmp = &((struct sockaddr_in *)if_addr_struct->ifa_addr)->sin_addr;
            /* 网络地址转换为字符串 */
            if (!inet_ntop(if_addr_struct->ifa_addr->sa_family, tmp, conf.host_ip, sizeof(conf.host_ip)))
                do_debug(LOG_FATAL, "get host ip error!\n");
        }
        if_addr_struct = if_addr_struct->ifa_next;
    }
}

/* 解析有效行 */
static int parse_line(char *buff) {
    char *token;

    if (!(token = strtok(buff, W_SPACE)))
        (void) 0;
    else if (strstr(buff, "mod_"))
        parse_mod(token);
    else if (strstr(buff, "spec_"))
        special_mod(token);
    else if (!strcmp(token, "output_interface"))
        parse_string(conf.output_interface);
    else if (!strcmp(token, "output_file_path"))
        parse_string(conf.output_file_path);
    else if (!strcmp(token, "module_path"))
        parse_string(conf.module_path);
    else if (!strcmp(token, "output_db_addr"))
        parse_string(conf.output_db_addr);
    else if (!strcmp(token, "output_db_pawd"))
        parse_string(conf.output_db_pawd);
    else if (!strcmp(token, "output_db_mod"))
        parse_add_string(conf.output_db_mod);
    else if (!strcmp(token, "output_nagios_mod"))
        parse_add_string(conf.output_nagios_mod);
    else if (!strcmp(token, "output_stdio_mod"))
        parse_add_string(conf.output_stdio_mod);
    else if (!strcmp(token, "debug_leve"))
        set_debug_level();
    else if (!strcmp(token, "include"))
        get_include_conf();
    else if (!strcmp(token, "server_addr"))
        parse_string(conf.server_addr);
    else if (!strcmp(token, "server_port"))
        parse_int(conf.server_port);
    else if (!strcmp(token, "cycle_time"))
        parse_int(conf.cycle_time);
    else if (!strcmp(token, "send_nsca_cmd"))
        parse_string(conf.send_nsca_cmd);
    else if (!strcmp(token, "send_nsca_conf"))
        parse_string(conf.send_nsca_conf);
    else if (!strcmp(token, "threshold"))
        get_threshold();
    else if (!strlen(conf.host_name) && !strlen(conf.host_ip))
        get_host();
    else
        return 0;

    return 1;
}

/* tsar所有配置文件路径 */
void get_include_conf(void) {
    char *token = strtok(NULL, W_SPACE);
    char cmd[LEN_1024] = {0};
    char buf[LEN_1024] = {0};
    FILE *stream, *fp;
    char *tmp, *p;
    char config_input_line[LEN_1024] = {0};

    if (token) {
        sprintf(cmd, "ls %s 2>/dev/null", token);
        if (strchr(cmd, ';') || strchr(cmd, '|') || strchr(cmd, '&'))
            do_debug(LOG_ERR, "include formart Error:%s\n", cmd);
        stream = popen(cmd, "r");   /* 执行命令 */
        if (!stream) {
            do_debug(LOG_ERR, "popen failed. Error:%s\n", strerror(errno));
            return;
        }
        /* 执行命令后返回的结果 */
        while (fgets(buf, LEN_1024, stream)) {
            do_debug(LOG_INFO, "parse file %s\n", buf);
            p = buf;
            while (p) {
                if (*p == '\n' || *p == '\r') {
                    *p = '\0';
                    break;
                }
                p++;
            }
            if (!(fp = fopen(buf, "r"))) {
                do_debug(LOG_ERR, "Unable to open configuration file: %s Error msg: %s\n", buf, strerror(errno));
                continue;
            }
            /* 读执行命令后返回的文件 */
            while (fgets(config_input_line, LEN_1024, fp)) {
                if ((tmp = strchr(config_input_line, '\n')))  /* 回车 */
                    *tmp = '\0';
                if ((tmp = strchr(config_input_line, '\r')))  /* 换行 */
                    *tmp = '\0';
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
        if (pclose(stream) == -1)
            do_debug(LOG_WARN, "pclose error\n");
    }
}

/* 解析tsar.conf文件 */
void parse_config_file(const char *file_name) {
    FILE *fp;
    char config_input_line[LEN_1024] = {0};
    char *token;

    /* 打开tsar配置文件 */
    if (!(fp = fopen(file_name, "r")))
        do_debug(LOG_FATAL, "Unable open file:%s\n", file_name);

    /* 结构初始化 */
    memset(&conf, '\0', sizeof(conf));
    memset(&mods, '\0', sizeof(mods));
    memset(&statis, '\0', sizeof(statis));
    /* cycle_time,server_port配置内存空间 */
    conf.cycle_time = (int *)malloc(sizeof(int));
    conf.server_port = (int *)malloc(sizeof(int));
    conf.debug_level = LOG_ERR;
    conf.mod_num = 0;

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
