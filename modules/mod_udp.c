#include "../tsar.h"

static char *udp_usage = "    --udp               UDP traffic     (v4)";

static struct mod_info udp_info[] = {
    {"  idgm", DETAIL_BIT,  0,  STATS_SUB_INTER},
    {"  odgm", DETAIL_BIT,  0,  STATS_SUB_INTER},
    {"noport", DETAIL_BIT,  0,  STATS_SUB_INTER},
    {"idmerr", DETAIL_BIT,  0,  STATS_SUB_INTER},
};

struct stats_udp {
    unsigned long long InDatagrams;
    unsigned long long OutDatagrams;
    unsigned long long NoPorts;
    unsigned long long InErrors;
};

static void read_udp_stats(struct module *mod) {
    FILE *fp;
    char line[LEN_1024] = {0};
    char buf[LEN_1024] = {0};
    struct stats_udp st_udp;
    int sw = FALSE;

    memset(&st_udp, 0, sizeof(struct stats_udp));

    if (!(fp = fopen(NET_SNMP, "r"))) {
        return;
    }

    while (fgets(line, LEN_1024, fp)) {
        if (!strncmp(line, "Udp:", 4)) {
            if (sw) {
                sscanf(line + 4, "%llu %llu %llu %llu",
                        &st_udp.InDatagrams,
                        &st_udp.NoPorts,
                        &st_udp.InErrors,
                        &st_udp.OutDatagrams);
                break;
            } else {
                sw = TRUE;
            }
        }
    }

    fclose(fp);

    int pos = sprintf(buf, "%lld,%lld,%lld,%lld",
            st_udp.InDatagrams,
            st_udp.NoPorts,
            st_udp.InErrors,
            st_udp.OutDatagrams);
    buf[pos] = '\0';
    set_mod_record(mod, buf);
}

void mod_register(struct module *mod) {
    register_mod_fileds(mod, "--udp", udp_usage, udp_info, 4, read_udp_stats, NULL);
}
