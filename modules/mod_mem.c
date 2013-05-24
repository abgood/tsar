#include "../tsar.h"

char *mem_usage = "    --mem               Physical memory share (active, inactive, cached, free, wired)";

static struct mod_info mem_info[] = {
    {"  free", DETAIL_BIT,  0,  STATS_NULL},
    {"  used", DETAIL_BIT,  0,  STATS_NULL},
    {"  buff", DETAIL_BIT, 0,  STATS_NULL},
    {"  cach", DETAIL_BIT,  0,  STATS_NULL},
    {" total", DETAIL_BIT,  0,  STATS_NULL},
    {"  util", SUMMARY_BIT,  0,  STATS_NULL}
};

/* read mod->record to array */
static void read_mem_stats(struct module *mod) {
}

/* set module array */
static void set_mem_record(struct module *mod, double st_array[], U_64 pre_array[], U_64 cur_array[], int inter) {
}

void mod_register(struct module *mod) {
    register_mod_fileds(mod, "--mem", mem_usage, mem_info, 6, read_mem_stats, set_mem_record);
}
