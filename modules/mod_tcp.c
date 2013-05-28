#include "../tsar.h"

static char *tcp_usage = "    --tcp               TCP traffic     (v4)";

static struct mod_info tcp_info[] = {
    {"active", DETAIL_BIT,  0,  STATS_SUB_INTER},
    {"pasive", DETAIL_BIT,  0,  STATS_SUB_INTER},
    {"  iseg", DETAIL_BIT,  0,  STATS_SUB_INTER},
    {"outseg", DETAIL_BIT,  0,  STATS_SUB_INTER},
    {"retran", SUMMARY_BIT,  0,  STATS_SUB_INTER}
};

struct stats_tcp {
    unsigned long long ActiveOpens;
    unsigned long long PassiveOpens;
    unsigned long long InSegs;
    unsigned long long OutSegs;
    unsigned long long AttemptFails;
    unsigned long long EstabResets;
    unsigned long long RetransSegs;
    unsigned long long InErrs;
    unsigned long long OutRsts;
};

static void read_tcp_stats(struct module *mod) {
    FILE *fp;
    char line[LEN_1024] = {0};
    char buf[LEN_1024] = {0};
    /* 两行"Tcp:"开头的行, 初始值为FALSE */
    int sw = FALSE;
    struct stats_tcp st_tcp;

    if (!(fp = fopen(NET_SNMP, "r"))) {
        return;
    }

    while (fgets(line, LEN_1024, fp)) {
        if (!strncmp(line, "Tcp:", 4)) {
            if (sw) {
                sscanf(line + 4, "%*u %*u %*u %*d %llu %llu %llu %llu %*u %llu %llu %llu %llu %llu",
                        &st_tcp.ActiveOpens,
                        &st_tcp.PassiveOpens,
                        &st_tcp.AttemptFails,
                        &st_tcp.EstabResets,
                        &st_tcp.InSegs,
                        &st_tcp.OutSegs,
                        &st_tcp.RetransSegs,
                        &st_tcp.InErrs,
                        &st_tcp.OutRsts);
                break;
            } else {
                sw = TRUE;
            }
        }
    }

    fclose(fp);

    int pos = sprintf(buf, "%lld,%lld,%lld,%lld,%lld",
            st_tcp.ActiveOpens,
            st_tcp.PassiveOpens,
            st_tcp.InSegs,
            st_tcp.OutSegs,
            st_tcp.RetransSegs);
    buf[pos] = '\0';
    set_mod_record(mod, buf);
}

static void set_tcp_record(struct module *mod, double st_array[], U_64 pre_array[], U_64 cur_array[], int inter) {
    int i;
    for (i = 0; i < 4; i++) {
        if (cur_array[i] >= pre_array[i]) {
            st_array[i] = (cur_array[i] - pre_array[i]) * 1.0 / inter;
        }
    }

    /* 重传率 */
    if ((cur_array[4] >= pre_array[4]) && (cur_array[3] > pre_array[3])) {
        st_array[4] = (cur_array[4] - pre_array[4]) * 100.0 / (cur_array[3] - pre_array[3]);
    }

    if (st_array[4] > 100.0) {
        st_array[4] = 100.0;
    }
}

void mod_register(struct module *mod) {
    register_mod_fileds(mod, "--tcp", tcp_usage, tcp_info, 5, read_tcp_stats, set_tcp_record);
}
