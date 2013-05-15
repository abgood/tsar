#ifndef TSAR_H
#define TSAR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <dlfcn.h>
#include <getopt.h>
#include <unistd.h>
#include <netdb.h>
#include <ifaddrs.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "define.h"
#include "config.h"
#include "debug.h"
#include "framework.h"
#include "output_file.h"
#include "output_db.h"
#include "output_nagios.h"

struct statistic {
    int total_mod_num;      /* 启用模块数量 */
    time_t cur_time;        /* 当前时间 */
};

extern struct configure conf;
extern struct statistic statis;
extern struct module mods[MAX_MOD_NUM];

#endif
