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
        printf("%s", line);
    }
}

/* 设置cpu信息到st_array中 */
static void set_cpu_record(struct module *mod, double st_array[],
        U_64 pre_array[], U_64 cur_array[], int inter) {
    ;
}

void mod_register(struct module *mod) {
    register_mod_fileds(mod, "--cpu", cpu_usage, cpu_info, 9, read_cpu_stats, set_cpu_record);
}
