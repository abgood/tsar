/*
 * 日志打印
 */

#include "tsar.h"

void 
do_debug(log_level_t level, char *fmt, ...) {
    /* 打印可变参数 */
    if (level >= conf.debug_level) {
        va_list argp;

        va_start(argp, fmt);
        vfprintf(stderr, fmt, argp);
        va_end(argp);
    }

    /* 致命错误 */
    if (LOG_FATAL == level)
        exit(1);
}
