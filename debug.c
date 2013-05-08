/*
 * 调试程序
 */

#include "tsar.h"

/* 调试程序 */
void do_debug(log_level_t level, const char *fmt, ...) {
    if (level >= conf.debug_level) {
        va_list argp;

        va_start(argp, fmt);
        vfprintf(stderr, fmt, argp);
        va_end(argp);
    }

    /* 日志等级为log_fatal时退出程序 */
    if (level == LOG_FATAL)
        exit(1);
}
