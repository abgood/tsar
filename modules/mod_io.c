#include "../tsar.h"
#include <linux/major.h>

FILE *iofp;
char buffer[LEN_256] = {0};
unsigned int n_partitions;      /* number of partitions */
int print_device = 1;
int print_partition = 0;

static char *io_usage = "    --io                Linux I/O performance";

/* io struct */
struct mod_info io_info[] = {
    {" rrqms", DETAIL_BIT,  MERGE_SUM,  STATS_NULL},
    {" wrqms", DETAIL_BIT,  MERGE_SUM,  STATS_NULL},
    {"    rs", DETAIL_BIT,  MERGE_SUM,  STATS_NULL},
    {"    ws", DETAIL_BIT,  MERGE_SUM,  STATS_NULL},
    {" rsecs", DETAIL_BIT,  MERGE_SUM,  STATS_NULL},
    {" wsecs", DETAIL_BIT,  MERGE_SUM,  STATS_NULL},
    {"rqsize", DETAIL_BIT,  MERGE_AVG,  STATS_NULL},
    {"qusize", DETAIL_BIT,  MERGE_AVG,  STATS_NULL},
    {" await", DETAIL_BIT,  MERGE_AVG,  STATS_NULL},
    {" svctm", DETAIL_BIT,  MERGE_AVG,  STATS_NULL},
    {"  util", SUMMARY_BIT,  MERGE_AVG,  STATS_NULL}
};

/* part struct */
struct part_info {
    unsigned int major;     /* 主设备号 */
    unsigned int minor;     /* 次设备号 */
    char name[LEN_32];      /* 设备名称 */
} partition[MAX_PARTITIONS];

struct blkio_info {
    unsigned long long rd_ios;      /* Read I/O operations */
    unsigned long long rd_merges;   /* Reads merged */
    unsigned long long rd_sectors;  /* Sectors read */
    unsigned long long rd_ticks;    /* Time in queue + service for read */
    unsigned long long wr_ios;      /* Write I/O operations */
    unsigned long long wr_merges;   /* Writes merged */
    unsigned long long wr_sectors;  /* Sectors written */
    unsigned long long wr_ticks;    /* Time in queue + service for write */
    unsigned long long ticks;       /* Time of requests in queue */
    unsigned long long aveq;        /* Average queue length */
} new_blkio[MAX_PARTITIONS];
#define STATS_IO_SIZE (sizeof(struct blkio_info))

/* print error */
void handle_error(const char *string, int error) {
    if (error) {
        fputs("iostat: ", stderr);
        if (errno) {
            perror(string);
        } else {
            fprintf(stderr, "%s\n", string);
        }

        exit(EXIT_FAILURE);
    }
}

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

int printable(unsigned int major, unsigned int minor) {
    if (IDE_DISK_MAJOR(major)) {
        return (!(minor & 0x3F) && print_device)
            || ((minor & 0x3F) && print_partition);
    } else if (SCSI_DISK_MAJOR(major)) {
        return (!(minor & 0x0F) && print_device)
            || ((minor & 0x0F) && print_partition);
    } else if(COMPAQ_MAJOR(major)){
        return (!(minor & 0x0F) && print_device)
            || ((minor & 0x0F) && print_partition);
    } else if(DEVMAP_MAJOR == major){
        return 0;
    }
    return 1;   /* if uncertain, print it */
}

/* init partition */
static void initialize(void) {
    const char *scan_fmt = NULL;
    scan_fmt = "%4d %4d %31s %u";

    while (fgets(buffer, sizeof(buffer), iofp)) {
        unsigned int reads = 0;
        struct part_info curr;

        if (sscanf(buffer, scan_fmt, &curr.major, &curr.minor,
                    curr.name, &reads) == 4) {
            unsigned int p;

            for (p = 0; p < n_partitions && (partition[p].major != curr.major 
                    || partition[p].minor != curr.minor); p++);

            /* 找出所有磁盘号 */
            if (p == n_partitions && p < MAX_PARTITIONS) {
                if (reads && printable(curr.major, curr.minor)) {
                    partition[p] = curr;
                    n_partitions = p + 1;
                }
            }
        }
    }
}

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
                if (partition[p].major == curr.major
                        && partition[p].minor == curr.minor) {
                    new_blkio[p] = blkio;
                    break;
                }
            }
        }
    }
}

/* read mod->record into array */
static void read_io_stat(struct module *mod) {
    setlinebuf(stdout);

    /*open io file */
    iofp = fopen(IO_FILE, "r");
    /* handle error */
    handle_error("Can't open /proc/diskstats", !iofp);
    initialize();
    get_kernel_stats();
}

/* set st_array */
static void set_io_record(struct module *mod, double st_array[],
        U_64 pre_array[], U_64 cur_array[], int inter) {
}

void mod_register(struct module *mod) {
    register_mod_fileds(mod, "--io", io_usage, io_info, 11, read_io_stat, set_io_record);
}
