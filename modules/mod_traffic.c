#include "../tsar.h"

static char *traffic_usage = "    --traffic           Net traffic statistics";

static struct mod_info traffic_info[] = {
    {" bytin", DETAIL_BIT,  0,  STATS_SUB_INTER},
    {"bytout", DETAIL_BIT,  0,  STATS_SUB_INTER},
    {" pktin", SUMMARY_BIT,  0,  STATS_SUB_INTER},
    {"pktout", SUMMARY_BIT,  0,  STATS_SUB_INTER}
};

struct stats_traffic {
    unsigned long long bytein;
    unsigned long long byteout;
    unsigned long long pktin;
    unsigned long long pktout;
};

static void read_traffic_stats(struct module *mod) {
    FILE *fp;
    char line[LEN_4096] = {0};
    char buf[LEN_4096] = {0};
    struct stats_traffic total_st, cur_st;
    char *p = NULL;

    memset(&total_st, 0, sizeof(struct stats_traffic));
    memset(&cur_st, 0, sizeof(struct stats_traffic));
    
    if (!(fp = fopen(NET_DEV, "r"))) {
        return;
    }

    while (fgets(line, LEN_4096, fp)) {
        if (strstr(line, "eth") || strstr(line, "em")) {
            memset(&cur_st, 0, sizeof(cur_st));
            p = strchr(line, ':');
            sscanf(p + 1, "%llu %llu %*u %*u %*u %*u %*u %*u %llu %llu %*u %*u %*u %*u %*u %*u",
                    &cur_st.bytein,
                    &cur_st.pktin,
                    &cur_st.byteout,
                    &cur_st.pktout);

            total_st.bytein += cur_st.bytein;
            total_st.byteout += cur_st.byteout;
            total_st.pktin += cur_st.pktin;
            total_st.pktout += cur_st.pktout;
        }
    }

    int pos = sprintf(buf, "%lld,%lld,%lld,%lld",
            total_st.bytein,        /* 接收字节数 */
            total_st.byteout,       /* 发送字节数 */
            total_st.pktin,         /* 接收包数 */
            total_st.pktout);       /* 发送包数 */
    buf[pos] = '\0';
    set_mod_record(mod, buf);
    fclose(fp);
}

void mod_register(struct module *mod) {
    register_mod_fileds(mod, "--traffic", traffic_usage, traffic_info, 4, read_traffic_stats, NULL);
}
