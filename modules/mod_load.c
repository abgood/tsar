#include "../tsar.h"

static char *load_usage = "    --load              System Run Queue and load average";

static struct mod_info load_info[] = {
    {" load1", SUMMARY_BIT,  0,  STATS_NULL},
    {" load5", DETAIL_BIT,  0,  STATS_NULL},
    {"load15", DETAIL_BIT,  0,  STATS_NULL},
    {"  runq", DETAIL_BIT,  0,  STATS_NULL},
    {"  plit", DETAIL_BIT,  0,  STATS_NULL}
};

struct stats_load {
    unsigned long nr_running;
    unsigned int  load_avg_1;
    unsigned int  load_avg_5;
    unsigned int  load_avg_15;
    unsigned int  nr_threads;
};

static void read_load_stats(struct module *mod) {
    FILE *fp;
    char buf[LEN_4096] = {0};
    struct stats_load st_load;
    int load_tmp[3];

    memset(&st_load, 0, sizeof(struct stats_load));

    if (!(fp = fopen(LOADAVG, "r"))) {
        return;
    }

    if (fscanf(fp, "%d.%d %d.%d %d.%d %ld/%d %*d\n",
            &load_tmp[0], &st_load.load_avg_1,
            &load_tmp[1], &st_load.load_avg_5,
            &load_tmp[2], &st_load.load_avg_15,
            &st_load.nr_running,
            &st_load.nr_threads) != 8) {
        fclose(fp);
        return;
    }

    st_load.load_avg_1 += load_tmp[0] * 100;
    st_load.load_avg_5 += load_tmp[1] * 100;
    st_load.load_avg_15 += load_tmp[2] * 100;

    if (st_load.nr_running) {
        /* 不算上当前进程 */
        st_load.nr_running--;
    }

    int pos = sprintf(buf, "%u,%u,%u,%lu,%u",
            st_load.load_avg_1,
            st_load.load_avg_5,
            st_load.load_avg_15,
            st_load.nr_running,
            st_load.nr_threads);
    buf[pos] = '\0';
    set_mod_record(mod, buf);
    fclose(fp);
}

static void set_load_record(struct module *mod, double *st_array, U_64 *pre_array, U_64 *cur_array, int inter) {
    int i;
    for (i = 0; i < 3; i++) {
        st_array[i] = cur_array[i] / 100.0;
    }

    st_array[3] = cur_array[3];
    st_array[4] = cur_array[4];
}

void mod_register(struct module *mod) {
    register_mod_fileds(mod, "--loadavg", load_usage, load_info, 5, read_load_stats, set_load_record);
}
