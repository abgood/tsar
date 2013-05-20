#include "tsar.h"

/* mod->record输出到nagios */
void output_nagios(void) {
    int now_time;
    char s_time[LEN_64] = {0};
    struct module *mod;
    int i, j, k, l, result = 0;
    char output[LEN_4096] = {0};
    char output_err[LEN_4096] = {0};

    /* 当前整分时间 */
    now_time = statis.cur_time - statis.cur_time%60;
    /* 周期为整分 */
    if ((*conf.cycle_time == 0) || (now_time % *conf.cycle_time != 0))
        return;

    /* 非合并模式 */
    conf.print_merge = MERGE_NOT;

    /* 从文件里读记录并保存至st_array */
    if (get_st_array_from_file())
        return;

    /* 重新加载输出到nagios里的模块 */
    reload_modules(conf.output_nagios_mod);

    /* 当前时间 */
    sprintf(s_time, "%ld", time(NULL));

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (!mod->enable) 
            continue;
        else {
            /* st_flag在set_record里设置, 表明st_array里有数据 */
            if (!mod->st_flag)
                printf("name %s do nothing!\n", mod->name);
            else {
                char *s_token;
                double *st_array;

                char check[LEN_64] = {0};
                char opt[LEN_32] = {0};
                char *n_record = strdup(mod->record);
                char *token = strtok(n_record, ITEM_SPLIT);
                struct mod_info *info = mod->info;
                j = 0;

                while (token) {
                    memset(check, 0, sizeof(check));
                    /* 组合模块与各项的名字 */
                    strcat(check, mod->name + 4);
                    strcat(check, ".");
                    s_token = strstr(token, ITEM_SPSTART);
                    /* multi item */
                    if (s_token){
                        memset(opt, 0, sizeof(opt));
                        strncat(opt, token, s_token - token);
                        strcat(check, opt);
                        strcat(check, ".");
                    }

                    /* get value */
                    st_array = &mod->st_array[j * mod->n_col];
                    token = strtok(NULL, ITEM_SPLIT);
                    j++;

                    for (k = 0; k < mod->n_col; k++) {
                        char check_item[LEN_64] = {0};
                        char *p;

                        memcpy(check_item, check, LEN_64);
                        p = info[k].hdr;
                        while (*p == ' ') p++;
                        /* 组合module与各列的名字 */
                        strcat(check_item, p);

                        for (l = 0; l < conf.mod_num; l++) {
                            /* threshold的check_name与check_item进行比较 
                             * 把代表util的st_array保存到output里
                             */
                            if (!strcmp(conf.check_name[l], check_item)) {
                                char value[LEN_32] = {0};

                                /* 模块多项目组合时, threshold要分项目写 */
                                sprintf(value, "%0.02f", st_array[k]);
                                strcat(output, check_item);
                                strcat(output, "=");
                                strcat(output, value);
                                strcat(output, " ");

                                /* 阀值计算 */
                                    /* Critical min与max之间
                                     * result为2, 状态危险
                                     * result为0, 状态正常
                                     */
                                if (conf.cmin[l] != 0 && st_array[k] >= conf.cmin[l]) {
                                    if (conf.cmax[l] == 0 || (conf.cmax[l] != 0 && st_array[k] <= conf.cmax[l])) {
                                        result = 2;
                                        strcat(output_err, check_item);
                                        strcat(output_err, "=");
                                        strcat(output_err, value);
                                        strcat(output_err, " ");
                                        continue;
                                    }
                                }
                                    /* Warning min与max之间
                                     * result为1, 状态警告
                                     * result为0, 状态正常
                                     */
                                if (conf.wmin[l] != 0 && st_array[k] >= conf.wmin[l]) {
                                    if (conf.wmax[l] == 0 || (conf.wmax[l] != 0 && st_array[k] <= conf.wmax[l])) {
                                        if (result != 2)
                                            result = 1;
                                        strcat(output_err, check_item);
                                        strcat(output_err, "=");
                                        strcat(output_err, value);
                                        strcat(output_err, " ");
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    /* 组合nagios命令串,把数据写到nagios服务器上 */
    if (!strcmp(output_err, ""))
        strcat(output_err, "OK");

    char nagios_cmd[LEN_1024] = {0};
    sprintf(nagios_cmd, "echo \"%s;tsar;%d;%s|%s\"|%s -H %s -p %d -to 10 -d \";\" -c %s", conf.host_name, result, output_err, output, conf.send_nsca_cmd, conf.server_addr, *(conf.server_port), conf.send_nsca_conf);

    do_debug(LOG_DEBUG, "send to nagios:%s\n", nagios_cmd);

    // if (system(nagios_cmd) != 0)
    //     do_debug(LOG_WARN, "nsca run error:%s\n", nagios_cmd);

    printf("%s\n", nagios_cmd);
}
