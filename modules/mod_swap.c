#include "../tsar.h"

static char *swap_usage = "    --swap              swap usage";

static struct mod_info swap_info[] = {
    {" swpin", DETAIL_BIT,  0,  STATS_SUB_INTER},
    {"swpout", DETAIL_BIT,  0,  STATS_SUB_INTER}
};

struct stats_swap {
    unsigned long pswpin;
    unsigned long pswpout;
};

static void read_vmstat_swap(struct module *mod) {
    FILE *fp;
    char line[LEN_128], buf[LEN_4096];
    struct stats_swap st_swap;

    memset(buf, 0, LEN_4096);
    memset(&st_swap, 0, sizeof(struct stats_swap));

    /* open file */
    if (!(fp = fopen(VMSTAT, "r"))) {
        return;
    }

    /* read file */
    while (fgets(line, LEN_128, fp)) {
        if (!strncmp(line, "pswpin ", 7)) {
            /* Read number of swap pages brought in */
            sscanf(line + 7, "%lu", &st_swap.pswpin);

        } else if (!strncmp(line, "pswpout ", 8)) {
            /* Read number of swap pages brought out */
            sscanf(line + 8, "%lu", &st_swap.pswpout);
        }
    }

    int pos = sprintf(buf, "%ld,%ld", st_swap.pswpin, st_swap.pswpout);
    buf[pos] = '\0';
    set_mod_record(mod, buf);
    fclose(fp);
    return;
}

void mod_register(struct module *mod) {
    register_mod_fileds(mod, "--swap", swap_usage, swap_info, 2, read_vmstat_swap, NULL);
}
