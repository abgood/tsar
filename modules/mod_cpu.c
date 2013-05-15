#include "../tsar.h"

static const char *cpu_usage = "    --cpu               CPU share (user, system, interrupt, nice, & idle)";

/* cpu信息参数 */
static struct mod_info cpu_info[] = {
    {"  user", DETAIL_BIT,  0,  STATS_NULL},
    {"   sys", DETAIL_BIT,  0,  STATS_NULL},
    {"  wait", DETAIL_BIT,  0,  STATS_NULL},
    {"  hirq", DETAIL_BIT,  0,  STATS_NULL},
    {"  sirq", DETAIL_BIT,  0,  STATS_NULL},
    {"  util", SUMMARY_BIT,  0,  STATS_NULL},
    {"  nice", HIDE_BIT,  0,  STATS_NULL},
    {" steal", HIDE_BIT,  0,  STATS_NULL},
    {" guest", HIDE_BIT,  0,  STATS_NULL},
};

/* cpu信息存放 */
struct stats_cpu {
    unsigned long long cpu_user;
    unsigned long long cpu_nice;
    unsigned long long cpu_sys;
    unsigned long long cpu_idle;
    unsigned long long cpu_iowait;
    unsigned long long cpu_steal;
    unsigned long long cpu_hardirq;
    unsigned long long cpu_softirq;
    unsigned long long cpu_guest;
};

/* 读系统cpu信息 */
static void read_cpu_stats(struct module *mod) {
    char line[LEN_4096] = {0};
    char buf[LEN_4096] = {0};
    FILE *fp;
    struct stats_cpu st_cpu;

    memset(&st_cpu, 0, sizeof(struct stats_cpu));
    memset(buf, 0, LEN_4096);

    if (!(fp = fopen(STAT, "r")))
        return;
    while (fgets(line, LEN_4096, fp)) {
        /* line是以"cpu "开头的 */
        if (!strncmp(line, "cpu ", 4)) {
            sscanf(line + 5, "%llu %llu %llu %llu %llu %llu %llu %llu %llu",
                    &st_cpu.cpu_user,
                    &st_cpu.cpu_nice,
                    &st_cpu.cpu_sys,
                    &st_cpu.cpu_idle,
                    &st_cpu.cpu_iowait,
                    &st_cpu.cpu_hardirq,
                    &st_cpu.cpu_softirq,
                    &st_cpu.cpu_steal,
                    &st_cpu.cpu_guest);
        }
    }
    /* cpu_util = st_cpu.cpu_user + st_cpu.cpu_sys + st_cpu.cpu_hardirq + st_cpu.cpu_softirq */

    int pos = sprintf(buf, "%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu",
            /* the store order is not same as read procedure */
            st_cpu.cpu_user,
            st_cpu.cpu_sys,
            st_cpu.cpu_iowait,
            st_cpu.cpu_hardirq,
            st_cpu.cpu_softirq,
            st_cpu.cpu_idle,
            st_cpu.cpu_nice,
            st_cpu.cpu_steal,
            st_cpu.cpu_guest);

    buf[pos] = '\0';
    /* 设置module收集到的记录 */
    set_mod_record(mod, buf);
    if (fclose(fp) < 0)
        return;
}

/* 设置cpu信息到st_array中 */
static void set_cpu_record(struct module *mod, double st_array[],
        U_64 pre_array[], U_64 cur_array[], int inter) {
    ;
}

void mod_register(struct module *mod) {
    register_mod_fileds(mod, "--cpu", cpu_usage, cpu_info, 9, read_cpu_stats, set_cpu_record);
}
