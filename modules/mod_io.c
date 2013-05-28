#include "../tsar.h"
#include <linux/major.h>

/* io help info */
static char *io_usage = "    --io                Linux I/O performance";

/* io info struct */
struct mod_info io_info[] = {
    {" rrqms", DETAIL_BIT,  MERGE_SUM,  STATS_NULL},        /* 合并读完成次数 */
    {" wrqms", DETAIL_BIT,  MERGE_SUM,  STATS_NULL},        /* 合并写完成次数 */
    {"    rs", DETAIL_BIT,  MERGE_SUM,  STATS_NULL},        /* 读完成次数 */
    {"    ws", DETAIL_BIT,  MERGE_SUM,  STATS_NULL},        /* 写完成次数 */
    {" rsecs", DETAIL_BIT,  MERGE_SUM,  STATS_NULL},        /* 读扇区数 */
    {" wsecs", DETAIL_BIT,  MERGE_SUM,  STATS_NULL},        /* 写扇区数 */
    {"rqsize", DETAIL_BIT,  MERGE_AVG,  STATS_NULL},        /* 读写总次数 */
    {"qusize", DETAIL_BIT,  MERGE_AVG,  STATS_NULL},        /* 平均队列长度 */
    {" await", DETAIL_BIT,  MERGE_AVG,  STATS_NULL},        /* 平均等待时间 */
    {" svctm", DETAIL_BIT,  MERGE_AVG,  STATS_NULL},        /* 平均服务时间 */
    {"  util", SUMMARY_BIT,  MERGE_AVG,  STATS_NULL}        /* disk util */
};

FILE *iofp;
char buffer[LEN_256] = {0};
unsigned int n_partitions;
int print_partition = 0;
int print_device = 1;

#define IO_FILE "/proc/diskstats"
#define MAX_PARTITIONS 16

#ifndef IDE_DISK_MAJOR
#define IDE_DISK_MAJOR(M) ((M) == IDE0_MAJOR || (M) == IDE1_MAJOR || \
        (M) == IDE2_MAJOR || (M) == IDE3_MAJOR || \
        (M) == IDE4_MAJOR || (M) == IDE5_MAJOR || \
        (M) == IDE6_MAJOR || (M) == IDE7_MAJOR || \
        (M) == IDE8_MAJOR || (M) == IDE9_MAJOR)
#endif  /* !IDE_DISK_MAJOR */

#ifndef SCSI_DISK_MAJOR
#ifndef SCSI_DISK8_MAJOR
#define SCSI_DISK8_MAJOR 128
#endif
#ifndef SCSI_DISK15_MAJOR
#define SCSI_DISK15_MAJOR 135
#endif
#define SCSI_DISK_MAJOR(M) ((M) == SCSI_DISK0_MAJOR || \
        ((M) >= SCSI_DISK1_MAJOR && \
         (M) <= SCSI_DISK7_MAJOR) || \
        ((M) >= SCSI_DISK8_MAJOR && \
         (M) <= SCSI_DISK15_MAJOR))
#endif  /* !SCSI_DISK_MAJOR */

#ifndef DEVMAP_MAJOR
#define DEVMAP_MAJOR 253
#endif

#ifndef COMPAQ_MAJOR
#define COMPAQ_CISS_MAJOR   104
#define COMPAQ_CISS_MAJOR7  111
#define COMPAQ_SMART2_MAJOR 72
#define COMPAQ_SMART2_MAJOR7    79
#define COMPAQ_MAJOR(M) (((M) >= COMPAQ_CISS_MAJOR && \
            (M) <= COMPAQ_CISS_MAJOR7) || \
        ((M) >= COMPAQ_SMART2_MAJOR && \
         (M) <= COMPAQ_SMART2_MAJOR7))
#endif /* !COMPAQ_MAJOR */

/* partition info */
struct part_info {
    unsigned int major;     /* 主设备号 */
    unsigned int minor;     /* 次设备号 */
    char name[LEN_32];      /* 设备名称 */
} partition[MAX_PARTITIONS];

/* block io info */
struct blkio_info {
    unsigned long long rd_ios;          /* 读磁盘的次数,成功完成读的总次数 */
    unsigned long long rd_merges;       /* 合并读完成次数 */
    unsigned long long rd_sectors;      /* 读扇区的次数,成功读过的扇区总次数 */
    unsigned long long rd_ticks;        /* 读花费的毫秒数 */
    unsigned long long wr_ios;          /* 写完成次数 */
    unsigned long long wr_merges;       /* 合并写完成次数 */
    unsigned long long wr_sectors;      /* 写扇区次数 */
    unsigned long long wr_ticks;        /* 写操作花费的毫秒数 */
    unsigned long long ticks;           /* 队列请求时间 */
    unsigned long long aveq;            /* 平均队列长度 */
} new_blkio[MAX_PARTITIONS];
#define STATS_IO_SIZE (sizeof(struct blkio_info))

/* throw error */
void handle_error(const char *string, int error) {
    if (error) {
        fputs("iostat: ", stderr);
        if (errno)
            perror(string);
        else
            fprintf(stderr, "%s\n", string);
        exit(EXIT_FAILURE);
    }
}

/* 通过主从设备号来识别设备 */
int printable(unsigned int major, unsigned int minor) {
    if (IDE_DISK_MAJOR(major)) {
        return (!(minor & 0x3F) && print_device) || ((minor & 0x3F) && print_partition);
    } else if (SCSI_DISK_MAJOR(major)) {
        return (!(minor & 0x0F) && print_device) || ((minor & 0x0F) && print_partition);    /* 找出磁盘不找分区 */
    } else if(COMPAQ_MAJOR(major)){
        return (!(minor & 0x0F) && print_device) || ((minor & 0x0F) && print_partition);
    } else if(DEVMAP_MAJOR == major){
        return 0;
    }
    return 1;   /* if uncertain, print it */
}

/* get partition name, check error match list */
void initialize(void) {
    const char *scan_fmt = NULL;
    scan_fmt = "%4d %4d %31s %u";
    /* read one line from /proc/diskstats */
    while (fgets(buffer, sizeof(buffer), iofp)) {
        unsigned int reads = 0;
        struct part_info curr;

        /* reads可读磁盘 */
        if (sscanf(buffer, scan_fmt, &curr.major, &curr.minor, curr.name, &reads) == 4) {
            unsigned int p;

            /* 去重找新 */
            for (p = 0; p < n_partitions && (partition[p].major != curr.major || partition[p].minor != curr.minor); p++);

            /* 把正常的part_info保存到partition[]里 */
            if (p == n_partitions && p < MAX_PARTITIONS) {
                if (reads && printable(curr.major, curr.minor)) {
                    partition[p] = curr;
                    n_partitions = p + 1;
                }
            }
        }
    }
}

/* get kernel status */
void get_kernel_stats(void) {
    const char *scan_fmt = NULL;
    scan_fmt = "%4d %4d %*s %u %u %llu %u %u %u %llu %u %*u %u %u";
    rewind(iofp);

    while (fgets(buffer, sizeof(buffer), iofp)) {
        int items;
        struct part_info curr;
        struct blkio_info blkio;
        memset(&blkio, 0, STATS_IO_SIZE);
        items = sscanf(buffer, scan_fmt,
                &curr.major, &curr.minor,
                &blkio.rd_ios, &blkio.rd_merges,
                &blkio.rd_sectors, &blkio.rd_ticks,
                &blkio.wr_ios, &blkio.wr_merges,
                &blkio.wr_sectors, &blkio.wr_ticks,
                &blkio.ticks, &blkio.aveq);

        /*
         * Unfortunately, we can report only transfer rates
         * for partitions in 2.6 kernels, all other I/O
         * statistics are unavailable.
         */
        if (items == 6) {
            blkio.rd_sectors = blkio.rd_merges;
            blkio.wr_sectors = blkio.rd_ticks;
            blkio.rd_ios = 0;
            blkio.rd_merges = 0;
            blkio.rd_ticks = 0;
            blkio.wr_ios = 0;
            blkio.wr_merges = 0;
            blkio.wr_ticks = 0;
            blkio.ticks = 0;
            blkio.aveq = 0;
            items = 12;
        }

        if (items == 12) {
            unsigned int p;

            /* Locate partition in data table */
            for (p = 0; p < n_partitions; p++) {
                if (partition[p].major == curr.major && partition[p].minor == curr.minor) {
                    new_blkio[p] = blkio;
                    break;
                }
            }
        }
    }
}

void print_partition_stats(struct module *mod)
{
    int pos = 0;
    char buf[LEN_4096];
    memset(buf, 0, LEN_4096);
    unsigned int p;

    for (p = 0; p < n_partitions; p++) {
        pos += sprintf(buf + pos, "%s=%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%d",
                partition[p].name,
                new_blkio[p].rd_ios,
                new_blkio[p].rd_merges,
                new_blkio[p].rd_sectors,
                new_blkio[p].rd_ticks,
                new_blkio[p].wr_ios,
                new_blkio[p].wr_merges,
                new_blkio[p].wr_sectors,
                new_blkio[p].wr_ticks,
                new_blkio[p].ticks,
                new_blkio[p].aveq,
                pos);
        pos += sprintf(buf + pos, ITEM_SPLIT);
    }
    if (pos) {
        buf[pos] = '\0';
        set_mod_record(mod, buf);
    }
    rewind(iofp);
    if (NULL != iofp) {
        if (fclose(iofp) < 0) {
            return;
        }
        iofp =NULL;
    }
    return;
}

/* read io info to array */
static void read_io_stat(struct module *mod) {
    setlinebuf(stdout);
    /* open io file */
    iofp = fopen(IO_FILE, "r");
    /* throw error */
    handle_error("Can't open /proc/diskstats", !iofp);
    /* init */
    initialize();
    /* get kernel status */
    get_kernel_stats();
    print_partition_stats(mod);
}

/* set st_array */
static void set_io_record(struct module *mod, double st_array[], U_64 pre_array[], U_64 cur_array[], int inter) {
    int i;
    for (i = 0; i < 11; i++) {
        if (cur_array[i] < pre_array[i]) {
            pre_array[i] = cur_array[i];
        }
    }

    unsigned long long rd_ios = cur_array[0] - pre_array[0];
    unsigned long long rd_merges = cur_array[1] - pre_array[1];
    unsigned long long rd_sectors = cur_array[2] - pre_array[2];
    unsigned long long rd_ticks = cur_array[3] - pre_array[3];
    unsigned long long wr_ios = cur_array[4] - pre_array[4];
    unsigned long long wr_merges = cur_array[5] - pre_array[5];
    unsigned long long wr_sectors = cur_array[6] - pre_array[6];
    unsigned long long wr_ticks = cur_array[7] - pre_array[7];
    unsigned long long ticks = cur_array[8] - pre_array[8];
    unsigned long long aveq = cur_array[9] - pre_array[9];

    double n_ios = rd_ios + wr_ios;
    double n_ticks = rd_ticks + wr_ticks;
    double n_kbytes = (rd_sectors + wr_sectors) / 2;

    st_array[0] = rd_merges / (inter * 1.0);
    st_array[1] = wr_merges / (inter * 1.0);
    st_array[2] = rd_ios / (inter * 1.0);
    st_array[3] = wr_ios / (inter * 1.0);
    st_array[4] = rd_sectors / (inter * 2.0);
    st_array[5] = wr_sectors / (inter * 2.0);
    st_array[6] = n_ios ? n_kbytes / n_ios : 0.0;
    st_array[7] = aveq / (inter * 1000);
    st_array[8] = n_ios ? n_ticks / n_ios : 0.0;
    st_array[9] = n_ios ? ticks / n_ios : 0.0;
    st_array[10] = ticks / (inter * 10.0);

    if (st_array[10] > 100.0) {
        st_array[10] = 100.0;
    }
}

/* module register */
void mod_register(struct module *mod) {
    register_mod_fileds(mod, "--io", io_usage, io_info, 11, read_io_stat, set_io_record);
}
