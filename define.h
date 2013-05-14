#ifndef TSAR_DEFINE_H
#define TSAR_DEFINE_H

/* 相关文件 */
#define DEFAULT_CONF_FILE "tsar.conf"   /* tsar配置文件 */
#define STAT "/proc/stat"               /* cpu信息文件 */
#define DEFAULT_CONF_FILE_PATH "tsar.conf"      /* tsar默认配置文件地址 */

/* 类型长度 */
#define U_64 unsigned long long

/* bool值 */
#define TRUE 1
#define FALSE 0

/* 长度 */
#define LEN_32 32
#define LEN_56 64
#define LEN_128 128
#define LEN_256 256
#define LEN_512 512
#define LEN_1024 1024
#define LEN_4096 4096
#define LEN_10240 10240

/* 字符分割 */
#define SECTION_SPLIT "|"   /* 模块间分割符 */
#define ITEM_SPLIT ";"      /* 模块内项目分割符 */
#define W_SPACE " \t\r\n"   /* tsar配置文件分割符 */
#define DATA_SPLIT ","      /* argv中要打印模块分隔符 */

/* 最大限制数 */
#define MAX_COL_NUM 64  /* 模块最大列数 */
#define MAX_MOD_NUM 32  /* 最多模块数 */

/* 默认打印值 */
#define DEFAULT_PRINT_INTERVAL 5        /* 默认打印间隔秒 */

/* 模块各项显示状态 */
enum {
    HIDE_BIT,           /* 隐藏位 */
    DETAIL_BIT,         /* 详细位 */
    SUMMARY_BIT,        /* 简要位 */
    SPEC_BIT            /* 特殊位 */
};            
             
/* 模块各项xx状态 */   
enum {            
    STATS_NULL,         /**/
    STATS_SUB,          /**/
    STATS_SUB_INTER     /**/
};                    
                     
/* 运行模式 */      
enum {             
    RUN_NULL,           /* 无运行模式 */
    RUN_LIST,           /* 列表运行 */
    RUN_CRON,           /* 计划任务运行 */
    RUN_CHECK_NEW,      /* 检查运行 */
    RUN_PRINT,          /* 打印运行 */
    RUN_PRINT_LIVE      /* 实时打印运行 */
};               
                
/* 模块内的多项目合并模式 */
enum {                 
    MERGE_NOT,          /* 非合并 */
    MERGE_ITEM          /* 合并 */
};

/* 数据显示模式, 简要 or 详细*/
enum {
    DATA_NULL,          /* 无显示模式 */
    DATA_SUMMARY,       /* 简要 */
    DATA_DETAIL,        /* 详细 */
    DATA_ALL            /* 所有 */
};

#endif
