#ifndef TSAR_DEBUG_H
#define TSAR_DEBUG_H

typedef enum {
    LOG_INFO,       /* 信息模式 */
    LOG_DEBUG,      /* 调试模式 */
    LOG_WARN,       /* 警告模式 */
    LOG_ERR,        /* 错误模式 */
    LOG_FATAL       /* 致命模式 */
} log_level_t;

void do_debug(log_level_t, char *, ...);

#endif
