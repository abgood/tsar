#include "../tsar.h"
#include <sys/statfs.h>

/* 模块介绍 */
char *partition_usage = "    --partition         Disk and partition usage";

/* 模块信息结构 */
struct mod_info part_info[] = {
    {" bfree", DETAIL_BIT,  MERGE_SUM,  STATS_NULL},
    {" bused", DETAIL_BIT,  MERGE_SUM,  STATS_NULL},
    {" btotl", DETAIL_BIT,  MERGE_SUM,  STATS_NULL},
    // {"  util", SUMMARY_BIT, MERGE_AVG,  STATS_NULL},
    {"  util", DETAIL_BIT, MERGE_AVG,  STATS_NULL},
};

#define MAXPART 20

/* 分区信息结构 */
struct stats_partition {
    int bsize;                          /* 块大小 */
    unsigned long long blocks;          /* 总块数 */
    unsigned long long bfree;           /* 可用块数 */
    unsigned long long bavail;          /* 非超级用户可获取的块数 */
    unsigned long long itotal;          /* 文件结点总数 */
    unsigned long long ifree;           /* 可用文件结点数 */
};

/* 读分区信息,并保存至结构对应成员里 */
int __read_partition_stat(char *fsname, struct stats_partition *sp) {
    struct statfs fsbuf;
    /* 查询文件系统相关信息并保存至fsbuf */
    if (!statfs(fsname, &fsbuf)) {
        sp->bsize = fsbuf.f_bsize;
        sp->blocks = fsbuf.f_blocks;
        sp->bfree = fsbuf.f_bfree;
        sp->bavail = fsbuf.f_bavail;
        sp->itotal = fsbuf.f_files;
        sp->ifree = fsbuf.f_ffree;
    }
    return 0;
}

/* 整理单个磁盘信息 */
int store_single_partition(char *buf, char *mntpath, struct stats_partition *sp) {
    int len = 0;
    len += sprintf(buf, "%s=", mntpath);        /* 硬盘挂载目录名 ==> buf */
    len += sprintf(buf + len, "%d,%lld,%lld,%lld",
            sp->bsize,      /* 块大小 */
            sp->bfree,      /* 可用块数 */
            sp->blocks,     /* 总块数 */
            sp->bavail);    /* 非超级用户可获取的块数 */
    return len;
}

/* 收集硬盘信息 */
void read_partition_stat(struct module *mod) {
    FILE *mntfile;
    int part_nr = 0;
    int pos = 0;
    struct mntent *mnt = NULL;
    struct stats_partition temp;
    char buf[LEN_4096] = {0};

    /* 打开挂载点项目文件 */
    mntfile = setmntent("/etc/mtab", "r");
    /* 循环读取挂载点, 返回挂载结构 */
    while ((mnt = getmntent(mntfile))) {
        if (! strncmp(mnt->mnt_fsname, "/", 1)) {
            if (part_nr >= MAXPART) break;
            /* read each partition infomation */
            __read_partition_stat(mnt->mnt_dir, &temp);

            /* 循环读取单块硬盘信息保存至buf */
            pos += store_single_partition(buf + pos, mnt->mnt_dir, &temp);
            pos += sprintf(buf + pos, ITEM_SPLIT);

            /* 硬盘计数, 最多挂载20块硬盘 */
            part_nr++;
        }
    }
    /* 关闭打开文件 */
    endmntent(mntfile);
    buf[pos] = '\0';
    set_mod_record(mod, buf);
    return;
}

/* 设置收集到的硬盘信息到st_array中 */
static void set_part_record(struct module *mod, double st_array[], U_64 pre_array[], U_64 cur_array[], int inter) {
    st_array[0] = cur_array[3] * cur_array[0];                      /* 非超级用户可获取的总的块大小 */
    st_array[1] = (cur_array[2] - cur_array[1]) * cur_array[0];     /* 已经被用的总的块大小 */
    st_array[2] = cur_array[2] * cur_array[0];                      /* 总的块大小 */

    U_64 used = cur_array[2] - cur_array[1];        /* 被用块数 */
    U_64 nonroot_total = cur_array[2] - cur_array[1] + cur_array[3];    /* root不能使用的总块数 */

    if (nonroot_total != 0)
        st_array[3] = (used * 100.0) / nonroot_total + ((used * 100) % nonroot_total != 0);

}

void mod_register(struct module *mod) {
    register_mod_fileds(mod, "--partition", partition_usage, part_info, 4, read_partition_stat, set_part_record);
}
